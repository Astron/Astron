#include "MessageDirector.h"
#include <algorithm>
#include <functional>
#include <boost/icl/interval_bounds.hpp>

#include "core/global.h"
#include "core/msgtypes.h"
#include "config/ConfigVariable.h"
#include "config/constraints.h"
#include "net/TcpAcceptor.h"
#include "MDNetworkParticipant.h"
#include "MDNetworkUpstream.h"

static ConfigGroup md_config("messagedirector");
static ConfigVariable<std::string> bind_addr("bind", "unspecified", md_config);
static ConfigVariable<std::string> connect_addr("connect", "unspecified", md_config);
static ValidAddressConstraint valid_bind_addr(bind_addr);
static ValidAddressConstraint valid_connect_addr(connect_addr);
static ConfigVariable<bool> threaded_mode("threaded", true, md_config);

static ConfigGroup daemon_config("daemon");
static ConfigVariable<std::string> daemon_name("name", "<unnamed>", daemon_config);
static ConfigVariable<std::string> daemon_url("url", "", daemon_config);

MessageDirector MessageDirector::singleton;


MessageDirector::MessageDirector() :  m_initialized(false), m_net_acceptor(nullptr), m_upstream(nullptr),
    m_shutdown(false), m_main_is_routing(false), m_thread(nullptr), m_log("msgdir", "Message Director")
{
}

MessageDirector::~MessageDirector()
{
    shutdown_threading();

    m_terminated_participants.insert(m_participants.begin(), m_participants.end());
    m_participants.clear();

    process_terminates();
}

void MessageDirector::init_network()
{
    assert(std::this_thread::get_id() == g_main_thread_id);

    if(!m_initialized) {
        // Bind to port and listen for downstream servers
        if(bind_addr.get_val() != "unspecified") {
            m_log.info() << "Opening listening socket..." << std::endl;

            TcpAcceptorCallback callback = std::bind(&MessageDirector::handle_connection,
                                           this, std::placeholders::_1);

            AcceptorErrorCallback err_callback = std::bind(&MessageDirector::handle_error,
                                                    this, std::placeholders::_1);

            m_net_acceptor = std::unique_ptr<TcpAcceptor>(new TcpAcceptor(callback, err_callback));
            m_net_acceptor->bind(bind_addr.get_val(), 7199);
            m_net_acceptor->start();
        }

        // Connect to upstream server and start handling received messages
        if(connect_addr.get_val() != "unspecified") {
            m_log.info() << "Connecting upstream..." << std::endl;

            MDNetworkUpstream *upstream = new MDNetworkUpstream(this);

            upstream->connect(connect_addr.get_val());

            m_upstream = upstream;
        }

        if(threaded_mode.get_val()) {
            m_thread.reset(new std::thread(std::bind(&MessageDirector::routing_thread, this)));
        }

        m_initialized = true;
    }
}

void MessageDirector::shutdown_threading()
{
    if(!m_thread) {
        return;
    }

    // Signal routing thread to shut down:
    {
        std::lock_guard<std::mutex> lock(m_messages_lock);
        m_shutdown = true;
        m_cv.notify_one();
    }

    // Wait for it to do so:
    m_thread->join();
    m_thread.reset();
}

void MessageDirector::route_datagram(MDParticipantInterface *p, DatagramHandle dg)
{
    std::unique_lock<std::mutex> lock(m_messages_lock);

    // We need to hold the lock to push the datagram into the message queue, and to signal the MD thread (if in threaded mode).
    m_messages.push(std::make_pair(p, dg));

    if(m_thread) {
        // If in threaded mode, ring the bell to let the MD thread know we have a datagram to process and return.
        m_cv.notify_one();
        return;
    }

    // We don't need to hold the message lock past this point, as we're not in threaded mode.
    lock.unlock();

    if(std::this_thread::get_id() != g_main_thread_id) {
        // We aren't working in threaded mode, but we aren't in the main thread
        // either. For safety, we should post this down to the main thread.
        TaskQueue::singleton.enqueue_task([self = this]() {
            self->flush_queue();
        });
    } else {
        // Main thread: Invoke flush_queue directly.
        flush_queue();
    }
}

void MessageDirector::flush_queue()
{
    // We want to be sure this is being invoked from within the main thread.
    assert(std::this_thread::get_id() == g_main_thread_id);

    if(m_main_is_routing) {
        // We're already in the middle of a queue flush, return immediately.
        return;
    }

    m_main_is_routing = true;
    
    {
        std::unique_lock<std::mutex> lock(m_messages_lock);

        while(!m_messages.empty()) {
            // Get and process the datagram:
            auto msg = m_messages.front();
            m_messages.pop();

            lock.unlock();
            process_datagram(msg.first, msg.second);
            lock.lock();
        }
    }

    // We're done flushing, we can now be invoked from others.
    m_main_is_routing = false;
}

// This function runs in a thread; it loops until it's told to shut down:
void MessageDirector::routing_thread()
{
    std::unique_lock<std::mutex> lock(m_messages_lock);

    while(true) {
        // Wait for something interesting to handle...
        while(m_messages.empty() && !m_shutdown) {
            m_cv.wait(lock);
        }

        if(m_shutdown) {
            // Ehh, we've been told to shut off:
            return;
        }

        // Get and process the message:
        auto msg = m_messages.front();
        m_messages.pop();

        lock.unlock();
        process_datagram(msg.first, msg.second);
        lock.lock();
    }
}

void MessageDirector::process_datagram(MDParticipantInterface *p, DatagramHandle dg)
{
    m_log.trace() << "Processing datagram...." << std::endl;

    std::vector<channel_t> channels;
    DatagramIterator dgi(dg);
    try {
        // Unpack channels to send messages to
        uint8_t channel_count = dgi.read_uint8();
        auto receive_log = m_log.trace();
        receive_log << "Receivers: ";
        for(uint8_t i = 0; i < channel_count; ++i) {
            channel_t channel = dgi.read_channel();
            receive_log << channel << ", ";
            channels.push_back(channel);
        }
        receive_log << "\n";
    } catch(DatagramIteratorEOF &) {
        // Log error with receivers output
        if(p) {
            m_log.error() << "Detected truncated datagram reading header from '"
                          << p->m_name << "'.\n";
        } else {
            m_log.error() << "Detected truncated datagram reading header from "
                          "unknown participant.\n";
        }
        return;
    }

    // Find the participants that need to receive the message
    std::unordered_set<ChannelSubscriber*> receiving_participants;
    lookup_channels(channels, receiving_participants);
    if(p) {
        receiving_participants.erase(p);
    }

    // Send the datagram to each participant
    for(const auto& it : receiving_participants) {
        auto participant = static_cast<MDParticipantInterface *>(it);
        DatagramIterator msg_dgi(dg, dgi.tell());

        try {
            participant->handle_datagram(dg, msg_dgi);
        } catch(DatagramIteratorEOF &) {
            // Log error with receivers output
            m_log.error() << "Detected truncated datagram in handle_datagram for '"
                          << participant->m_name << "' from participant '" << p->m_name << "'.\n";
            return;
        }
    }

    // Send message upstream, if necessary
    if(p && m_upstream) {
        m_upstream->handle_datagram(dg);
        m_log.trace() << "...routing upstream." << std::endl;
    } else if(!p) {
        // If there is no participant, then it came from the upstream
        m_log.trace() << "...not routing upstream: It came from there." << std::endl;
    } else {
        // Otherwise this is the root MessageDirector.
        m_log.trace() << "...not routing upstream: There is none." << std::endl;
    }

    // N.B. Participants may reach end-of-life after receiving a datagram, or may
    // be terminated in another thread (for example if a network socket closes);
    // either way, process any received terminates after processing a datagram.
    process_terminates();
}


void MessageDirector::process_terminates()
{
    std::unordered_set<MDParticipantInterface*> terminating_participants;

    {
        std::lock_guard<std::mutex> lock(m_terminated_lock);
        terminating_participants = std::move(m_terminated_participants);
    }

    for(const auto& it : terminating_participants) {
        delete it;
    }
}

void MessageDirector::on_add_channel(channel_t c)
{
    if(m_upstream) {
        // Send upstream control message
        m_upstream->subscribe_channel(c);
    }
}

void MessageDirector::on_remove_channel(channel_t c)
{
    if(m_upstream) {
        // Send upstream control message
        m_upstream->unsubscribe_channel(c);
    }
}

void MessageDirector::on_add_range(channel_t lo, channel_t hi)
{
    if(m_upstream) {
        // Send upstream control message
        m_upstream->subscribe_range(lo, hi);
    }
}

void MessageDirector::on_remove_range(channel_t lo, channel_t hi)
{
    if(m_upstream) {
        // Send upstream control message
        m_upstream->unsubscribe_range(lo, hi);
    }
}

void MessageDirector::handle_connection(const std::shared_ptr<uvw::TcpHandle> &socket)
{
    uvw::Addr remote = socket->peer();
    m_log.info() << "Got an incoming connection from "
                 << remote.ip << ":" << remote.port << std::endl;
    new MDNetworkParticipant(socket); // It deletes itself when connection is lost
}

void MessageDirector::handle_error(const uvw::ErrorEvent& evt)
{
    if(evt.code() == UV_EADDRINUSE || evt.code() == UV_EADDRNOTAVAIL) {
        m_log.fatal() << "Failed to bind to address: " << evt.what() << "\n";
        exit(1);
    }
}

void MessageDirector::add_participant(MDParticipantInterface* p)
{
    std::lock_guard<std::mutex> lock(m_participants_lock);
    m_participants.insert(p);
}

void MessageDirector::remove_participant(MDParticipantInterface* p)
{
    // Unsubscribe the participant from any remaining channels
    unsubscribe_all(p);

    // Stop tracking participant
    {
        std::lock_guard<std::mutex> lock(m_participants_lock);
        m_participants.erase(p);
    }

    // Send out any post-remove messages the participant may have added.
    // N.B. this is done last, because we don't want to send messages
    // through the Director while a participant is being removed, as
    // certain data structures may not have their invariants satisfied
    // during that time.
    p->post_remove();

    // Mark the participant for deletion
    {
        std::lock_guard<std::mutex> lock(m_terminated_lock);
        m_terminated_participants.insert(p);
    }
}

void MessageDirector::preroute_post_remove(channel_t sender, DatagramHandle post_remove)
{
    // Add post remove upstream
    if(m_upstream != nullptr) {
        DatagramPtr dg = Datagram::create(CONTROL_ADD_POST_REMOVE);
        dg->add_channel(sender);
        dg->add_blob(post_remove);
        m_upstream->handle_datagram(dg);
    }
}

void MessageDirector::recall_post_removes(channel_t sender)
{
    // Clear post removes upstream
    if(m_upstream != nullptr) {
        DatagramPtr dg = Datagram::create(CONTROL_CLEAR_POST_REMOVES);
        dg->add_channel(sender);
        m_upstream->handle_datagram(dg);
    }
}

void MessageDirector::receive_datagram(DatagramHandle dg)
{
    route_datagram(nullptr, dg);
}

void MessageDirector::receive_disconnect(const uvw::ErrorEvent &evt)
{
    m_log.fatal() << "Lost connection to upstream md: " << evt.what() << std::endl;
    exit(1);
}
