#include "core/global.h"

#include "EventSender.h"
#include "net/address_utils.h"

EventSender::EventSender() : m_log("eventsender", "Event Sender"),
    m_socket(io_service, udp::v4()), m_enabled(false)
{

}

void EventSender::init(const std::string& target)
{
    if(target == "") {
        m_enabled = false;
        m_log.debug() << "Not enabled." << std::endl;
        return;
    }

    m_log.debug() << "Resolving target..." << std::endl;
    boost::system::error_code ec;
    auto addresses = resolve_address(target, 7197, io_service, ec);
    if(ec.value() != 0) {
        m_log.fatal() << "Couldn't resolve " << target << std::endl;
        exit(1);
    }

    m_target = udp::endpoint(addresses[0].address(), addresses[0].port());
    m_enabled = true;

    m_log.debug() << "Initialized." << std::endl;
}

void EventSender::send(DatagramHandle dg)
{
    if(!m_enabled) {
        m_log.trace() << "Disabled; discarding event..." << std::endl;
        return;
    }

    m_log.trace() << "Sending event..." << std::endl;
    m_socket.send_to(boost::asio::buffer(dg->get_data(), dg->size()),
                     m_target);
}

// And now the convenience class:
LoggedEvent::LoggedEvent()
{
    add("type", "unset");
    add("sender", "unset");
}

LoggedEvent::LoggedEvent(const std::string &type)
{
    add("type", type);
    add("sender", "unset");
}

LoggedEvent::LoggedEvent(const std::string &type, const std::string &sender)
{
    add("type", type);
    add("sender", sender);
}

void LoggedEvent::add(const std::string &key, const std::string &value)
{
    if(m_keys.find(key) == m_keys.end()) {
        m_keys[key] = m_kv.size();
        m_kv.push_back(std::make_pair(key, value));
    } else {
        m_kv[m_keys[key]] = std::make_pair(key, value);
    }
}

static inline void pack_string(DatagramPtr dg, const std::string &str)
{
    size_t size = str.size();

    if(size < 32) {
        // Small enough for fixstr:
        dg->add_uint8(0xa0 + size);
    } else {
        // Use a str16.
        // We don't have to worry about str32, nothing that big will fit in a
        // single UDP packet anyway.
        dg->add_uint8(0xda);
        dg->add_uint8(size >> 8 & 0xFF);
        dg->add_uint8(size & 0xFF);
    }

    dg->add_data(str);
}

DatagramHandle LoggedEvent::make_datagram() const
{
    DatagramPtr dg = Datagram::create();

    // First, append the size of our map:
    size_t size = m_kv.size();
    if(size < 16) {
        // Small enough for fixmap:
        dg->add_uint8(0x80 + size);
    } else {
        // Use a map16.
        // We don't have to worry about map32, nothing that big will fit in a
        // single UDP packet anyway.
        dg->add_uint8(0xde);
        dg->add_uint8(size >> 8 & 0xFF);
        dg->add_uint8(size & 0xFF);
    }

    for(auto &it : m_kv) {
        pack_string(dg, it.first);
        pack_string(dg, it.second);
    }

    return dg;
}
