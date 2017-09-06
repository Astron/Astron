#include <ctime>
#include <cctype>

#include "core/RoleFactory.h"
#include "config/constraints.h"
#include "EventLogger.h"
#include "util/EventSender.h"

#include "msgpack_decode.h"

static RoleConfigGroup el_config("eventlogger");
static ConfigVariable<std::string> bind_addr("bind", "0.0.0.0:7197", el_config);
static ConfigVariable<std::string> output_format("output", "events-%Y%m%d-%H%M%S.log", el_config);
static ConfigVariable<std::string> rotate_interval("rotate_interval", "0", el_config);
static ValidAddressConstraint valid_bind_addr(bind_addr);

EventLogger::EventLogger(RoleConfig roleconfig) : Role(roleconfig),
    m_log("eventlogger", "Event Logger"), m_file(nullptr)
{
    auto loop = uvw::Loop::getDefault();
    m_socket = loop->resource<uvw::UDPHandle>();

    bind(bind_addr.get_rval(roleconfig));

    m_file_format = output_format.get_rval(roleconfig);
    open_log();

    LoggedEvent event("log-opened", "EventLogger");
    event.add("msg", "Log opened upon Event Logger startup.");
    process_packet(event.make_datagram());

    start_receive();
}

void EventLogger::bind(const std::string &addr)
{
    m_log.info() << "Opening UDP socket..." << std::endl;
    auto address = resolve_address(addr, 7197);
    if(address.ip == "") {
        m_log.fatal() << "Couldn't resolve " << addr << std::endl;
        exit(1);
    }

    m_socket->bind(address);
    m_socket->on<uvw::UDPDataEvent>([this](const auto& event, auto&) {
        this->handle_receive(event);
    });
    m_socket->on<uvw::ErrorEvent>([this](const auto& error, auto&) {
        this->handle_error(error);
    });
}

void EventLogger::open_log()
{
    time_t rawtime;
    time(&rawtime);

    char filename[1024];
    strftime(filename, 1024, m_file_format.c_str(), localtime(&rawtime));
    m_log.debug() << "New log filename: " << filename << std::endl;

    if(m_file) {
        m_file->close();
    }

    m_file.reset(new std::ofstream(filename));

    m_log.info() << "Opened new log." << std::endl;
}

void EventLogger::cycle_log()
{
    open_log();

    LoggedEvent event("log-opened", "EventLogger");
    event.add("msg", "Log cycled.");
    process_packet(event.make_datagram());
}

void EventLogger::process_packet(DatagramHandle dg)
{
    DatagramIterator dgi(dg);
    std::stringstream stream;

    try {
        msgpack_decode(stream, dgi);
    } catch(DatagramIteratorEOF&) {
        m_log.error() << "Received truncated packet" << std::endl;
        return;
    }

    if(dgi.tell() != dg->size()) {
        m_log.error() << "Received packet with extraneous data" << std::endl;
        return;
    }

    std::string data = stream.str();
    m_log.trace() << "Received: " << data << std::endl;

    // This is a little bit of a kludge, but we should make sure we got a
    // MessagePack map as the event log element, and not some other type. The
    // easiest way to do this is to make sure that the JSON representation
    // begins with {
    if(data[0] != '{') {
        m_log.error() << "Received non-map event log: " << data << std::endl;
        return;
    }

    // Now let's insert our timestamp:
    time_t rawtime;
    time(&rawtime);

    char timestamp[64];
    strftime(timestamp, 64, "{\"_time\": \"%Y-%m-%d %H:%M:%S%z\", ", localtime(&rawtime));

    *m_file.get() << timestamp << data.substr(1) << std::endl;
}

void EventLogger::start_receive()
{
    m_socket->recv();
}

void EventLogger::handle_receive(const uvw::UDPDataEvent& event)
{
    m_log.trace() << "Got packet from "
                  << event.sender.ip << ":" << event.sender.port << std::endl;

    DatagramPtr dg = Datagram::create(std::string(event.data.get(), event.length));
    process_packet(dg);
}

void EventLogger::handle_error(const uvw::ErrorEvent& error)
{
    m_log.warning() << "While receiving packet: " << error.what() << std::endl;
}

static RoleFactoryItem<EventLogger> el_fact("eventlogger");
