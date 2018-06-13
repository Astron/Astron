#include "core/global.h"
#include "core/msgtypes.h"
#include "config/constraints.h"
#include "dclass/dc/Class.h"
#include <exception>
#include <stdexcept>

#include "DistributedObject.h"
#include "StateServer.h"

using dclass::Class;

static RoleConfigGroup stateserver_config("stateserver");
static ConfigVariable<channel_t> control_channel("control", INVALID_CHANNEL, stateserver_config);
static InvalidChannelConstraint control_not_invalid(control_channel);
static ReservedChannelConstraint control_not_reserved(control_channel);

StateServer::StateServer(RoleConfig roleconfig) : Role(roleconfig)
{
    init_metrics();

    channel_t channel = control_channel.get_rval(m_roleconfig);
    if(channel != INVALID_CHANNEL) {
        subscribe_channel(channel);
        subscribe_channel(BCHAN_STATESERVERS);

        std::stringstream name;
        name << "StateServer(" << channel << ")";
        m_log = std::unique_ptr<LogCategory>(new LogCategory("stateserver", name.str()));
        set_con_name(name.str());
    }
}

void StateServer::init_metrics()
{
    auto objs_builder = &prometheus::BuildGauge()
            .Name("objects")
            .Register(*g_registry);
    auto obj_size_builder = &prometheus::BuildHistogram()
            .Name("object_size")
            .Register(*g_registry);
    auto obj_creation_builder = &prometheus::BuildCounter()
            .Name("ss_creation_ctr")
            .Register(*g_registry);
    m_obj_creation_ctr = &obj_creation_builder->Add({});
    auto obj_deletion_builder = &prometheus::BuildCounter()
            .Name("ss_deletion_ctr")
            .Register(*g_registry);
    m_obj_deletion_ctr = &obj_deletion_builder->Add({});
    auto obj_query_builder = &prometheus::BuildCounter()
            .Name("ss_query_ctr")
            .Register(*g_registry);
    m_obj_query_ctr = &obj_query_builder->Add({});
    auto obj_change_builder = &prometheus::BuildCounter()
            .Name("ss_change_ctr")
            .Register(*g_registry);
    m_obj_change_ctr = &obj_change_builder->Add({});


    // Register gauges and histograms for each class in our dc file.
    for (unsigned int i = 0; i < g_dcf->get_num_classes(); i++) {
        auto dc_class = g_dcf->get_class(i);
        auto dc_id = dc_class->get_id();
        auto dc_name = dc_class->get_name();

        m_objs_gauges[dc_id] = &objs_builder->Add({{"dclass", dc_name}});

        m_obj_size_hist[dc_id] = &obj_size_builder->Add({{"dclass", dc_name}},
                                                                prometheus::Histogram::BucketBoundaries{0, 4, 16, 64, 256, 1024, 4096, 16384, 65536});
    }
}

void StateServer::handle_generate(DatagramIterator &dgi, bool has_other)
{
    doid_t do_id = dgi.read_doid();
    doid_t parent_id = dgi.read_doid();
    zone_t zone_id = dgi.read_zone();
    uint16_t dc_id = dgi.read_uint16();

    // Make sure the object id is unique
    if(m_objs.find(do_id) != m_objs.end()) {
        m_log->warning() << "Received generate for already-existing object ID=" << do_id << std::endl;
        return;
    }

    // Make sure the class exists in the file
    const Class *dc_class = g_dcf->get_class_by_id(dc_id);
    if(!dc_class) {
        m_log->error() << "Received create for unknown dclass with class id '" << dc_id << "'\n";
        return;
    }

    // Create the object
    DistributedObject *obj = nullptr;
    try {
        obj = new DistributedObject(this, do_id, parent_id, zone_id, dc_class, dgi, has_other);
    } catch(const DatagramIteratorEOF&) {
        m_log->error() << "Received truncated generate for "
                       << dc_class->get_name() << "(" << do_id << ")" << std::endl;
        return;
    }

    m_objs[do_id] = obj;

    // Log statistics about the new object to Prometheus.
    auto gauge = m_objs_gauges[dc_id];
    auto hist = m_obj_size_hist[dc_id];

    if(gauge)
        gauge->Increment();
    if(hist)
        hist->Observe(obj->size());
    if (m_obj_creation_ctr)
        m_obj_creation_ctr->Increment();
}

void StateServer::handle_delete_ai(DatagramIterator& dgi, channel_t sender)
{
    channel_t ai_channel = dgi.read_channel();
    std::unordered_set<channel_t> targets;
    for(const auto& it : m_objs) {
        if(it.second && it.second->m_ai_channel == ai_channel && it.second->m_ai_explicitly_set) {
            targets.insert(it.second->m_do_id);
        }
    }

    if(targets.size()) {
        DatagramPtr dg = Datagram::create(targets, sender, STATESERVER_DELETE_AI_OBJECTS);
        dg->add_channel(ai_channel);
        route_datagram(dg);
    }
}

void StateServer::handle_datagram(DatagramHandle, DatagramIterator &dgi)
{
    channel_t sender = dgi.read_channel();
    uint16_t msgtype = dgi.read_uint16();
    switch(msgtype) {
    case STATESERVER_CREATE_OBJECT_WITH_REQUIRED: {
        handle_generate(dgi, false);
        break;
    }
    case STATESERVER_CREATE_OBJECT_WITH_REQUIRED_OTHER: {
        handle_generate(dgi, true);
        break;
    }
    case STATESERVER_DELETE_AI_OBJECTS: {
        handle_delete_ai(dgi, sender);
        break;
    }
    default:
        m_log->warning() << "Received unknown message: msgtype=" << msgtype << std::endl;
    }
}

RoleFactoryItem<StateServer> ss_fact("stateserver");
