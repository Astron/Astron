#include "DatabaseBackend.h"
#include "DBBackendFactory.h"
#include "DatabaseServer.h"

#include "core/global.h"
#include "util/DatagramIterator.h"
#include "util/Datagram.h"

#include "dclass/dc/DistributedType.h"
#include "dclass/dc/ArrayType.h"
#include "dclass/dc/Struct.h"
#include "dclass/dc/Method.h"

#include "mongo/client/dbclient.h"
#include "mongo/bson/bson.h"

using namespace std;
using namespace mongo;

// TODO: This eventually needs to be a config variable. However, it's
// unique to this backend, so it doesn't belong in the DatabaseBackend.cpp
// file.
#define NUM_WORKERS 8

// These are helper functions to convert between BSONElement and packed Bamboo
// field values.

static BSONObj bamboo2bson(const dclass::DistributedType *type,
                           DatagramIterator &dgi)
{
    // The BSON library's weird data model doesn't allow elements to exist on
    // their own; they must be part of an object. Therefore, we always return
    // results in a single BSONObj with key "_"
    BSONObjBuilder b;
    switch(type->get_type()) {
    case dclass::Type::T_INT8: {
        b << "_" << dgi.read_int8();
    }
    break;
    case dclass::Type::T_INT16: {
        b << "_" << dgi.read_int16();
    }
    break;
    case dclass::Type::T_INT32: {
        b << "_" << dgi.read_int32();
    }
    break;
    case dclass::Type::T_INT64: {
        b.appendIntOrLL("_", dgi.read_int64());
    }
    break;
    case dclass::Type::T_UINT8: {
        b << "_" << dgi.read_uint8();
    }
    break;
    case dclass::Type::T_UINT16: {
        b << "_" << dgi.read_uint16();
    }
    break;
    case dclass::Type::T_UINT32: {
        b << "_" << dgi.read_uint32();
    }
    break;
    case dclass::Type::T_UINT64: {
        b.appendIntOrLL("_", dgi.read_uint64());
    }
    break;
    case dclass::Type::T_CHAR: {
        unsigned char c = dgi.read_uint8();
        string str(c, 1);
        b << "_" << str;
    }
    break;
    case dclass::Type::T_FLOAT32: {
        b << "_" << dgi.read_float32();
    }
    break;
    case dclass::Type::T_FLOAT64: {
        b << "_" << dgi.read_float64();
    }
    break;
    case dclass::Type::T_STRING: {
        vector<uint8_t> vec = dgi.read_data(type->get_size());
        string str((const char *)vec.data(), vec.size());
        b << "_" << str;
    }
    case dclass::Type::T_VARSTRING: {
        b << "_" << dgi.read_string();
    }
    break;
    case dclass::Type::T_BLOB: {
        vector<uint8_t> blob = dgi.read_data(type->get_size());
        b.appendBinData("_", blob.size(), BinDataGeneral, blob.data());
    }
    break;
    case dclass::Type::T_VARBLOB: {
        vector<uint8_t> blob = dgi.read_blob();
        b.appendBinData("_", blob.size(), BinDataGeneral, blob.data());
    }
    break;
    case dclass::Type::T_ARRAY: {
        const dclass::ArrayType *array = type->as_array();

        BSONArrayBuilder ab;

        for(size_t i = 0; i < array->get_array_size(); i++) {
            ab << bamboo2bson(array->get_element_type(), dgi)["_"];
        }

        b << "_" << ab.arr();
    }
    break;
    case dclass::Type::T_VARARRAY: {
        const dclass::ArrayType *array = type->as_array();

        dgsize_t array_length = dgi.read_size();
        dgsize_t starting_size = dgi.tell();

        BSONArrayBuilder ab;

        while(dgi.tell() != starting_size + array_length) {
            ab << bamboo2bson(array->get_element_type(), dgi)["_"];
            if(dgi.tell() > starting_size + array_length) {
                throw mongo::DBException("Discovered corrupt array-length tag!", 0);
            }
        }

        b << "_" << ab.arr();
    }
    break;
    case dclass::Type::T_STRUCT: {
        const dclass::Struct *s = type->as_struct();
        size_t fields = s->get_num_fields();
        BSONObjBuilder ob;

        for(unsigned int i = 0; i < fields; ++i) {
            const dclass::Field *field = s->get_field(i);
            ob << field->get_name() << bamboo2bson(field->get_type(), dgi)["_"];
        }

        b << "_" << ob.obj();
    }
    break;
    case dclass::Type::T_METHOD: {
        const dclass::Method *m = type->as_method();
        size_t parameters = m->get_num_parameters();
        BSONObjBuilder ob;

        for(unsigned int i = 0; i < parameters; ++i) {
            const dclass::Parameter *parameter = m->get_parameter(i);
            string name = parameter->get_name();
            if(name.empty()) {
                stringstream n;
                n << "_" << i;
                name = n.str();
            }
            ob << name << bamboo2bson(parameter->get_type(), dgi)["_"];
        }

        b << "_" << ob.obj();
    }
    break;
    case dclass::Type::T_INVALID:
    default:
        assert(false);
        break;
    }

    return b.obj();
}

static void bson2bamboo(const dclass::DistributedType *type,
                        const BSONElement &element,
                        Datagram &dg)
{
    switch(type->get_type()) {
    case dclass::Type::T_INT8: {
        dg.add_int8(element.Int());
    }
    break;
    case dclass::Type::T_INT16: {
        dg.add_int16(element.Int());
    }
    break;
    case dclass::Type::T_INT32: {
        dg.add_int32(element.Int());
    }
    break;
    case dclass::Type::T_INT64: {
        dg.add_int64(element.Int());
    }
    break;
    case dclass::Type::T_UINT8: {
        dg.add_uint8(element.Int());
    }
    break;
    case dclass::Type::T_UINT16: {
        dg.add_uint16(element.Int());
    }
    break;
    case dclass::Type::T_UINT32: {
        dg.add_uint32(element.Int());
    }
    break;
    case dclass::Type::T_UINT64: {
        dg.add_uint64(element.Int());
    }
    break;
    case dclass::Type::T_CHAR: {
        string str = element.String();
        if(str.size() != 1) {
            throw mongo::DBException("Expected single-length string for char field", 0);
        }
        dg.add_uint8(str[0]);
    }
    break;
    case dclass::Type::T_FLOAT32: {
        dg.add_float32(element.Number());
    }
    break;
    case dclass::Type::T_FLOAT64: {
        dg.add_float64(element.Number());
    }
    break;
    case dclass::Type::T_STRING: {
        dg.add_data(element.String());
    }
    break;
    case dclass::Type::T_VARSTRING: {
        dg.add_string(element.String());
    }
    break;
    case dclass::Type::T_BLOB: {
        int len;
        const uint8_t *rawdata = (const uint8_t *)element.binData(len);
        dg.add_data(rawdata, len);
    }
    break;
    case dclass::Type::T_VARBLOB: {
        int len;
        const uint8_t *rawdata = (const uint8_t *)element.binData(len);
        dg.add_blob(rawdata, len);
    }
    break;
    case dclass::Type::T_ARRAY: {
        const dclass::ArrayType *array = type->as_array();
        std::vector<BSONElement> data = element.Array();

        for(auto it = data.begin(); it != data.end(); ++it) {
            bson2bamboo(array->get_element_type(), *it, dg);
        }
    }
    break;
    case dclass::Type::T_VARARRAY: {
        const dclass::ArrayType *array = type->as_array();
        std::vector<BSONElement> data = element.Array();

        DatagramPtr newdg = Datagram::create();

        for(auto it = data.begin(); it != data.end(); ++it) {
            bson2bamboo(array->get_element_type(), *it, *newdg);
        }

        dg.add_blob(newdg->get_data(), newdg->size());
    }
    break;
    case dclass::Type::T_STRUCT: {
        const dclass::Struct *s = type->as_struct();
        size_t fields = s->get_num_fields();
        for(unsigned int i = 0; i < fields; ++i) {
            const dclass::Field *field = s->get_field(i);
            bson2bamboo(field->get_type(), element[field->get_name()], dg);
        }
    }
    break;
    case dclass::Type::T_METHOD: {
        const dclass::Method *m = type->as_method();
        size_t parameters = m->get_num_parameters();
        for(unsigned int i = 0; i < parameters; ++i) {
            const dclass::Parameter *parameter = m->get_parameter(i);
            string name = parameter->get_name();
            if(name.empty() || element[name].eoo()) {
                stringstream n;
                n << "_" << i;
                name = n.str();
            }
            bson2bamboo(parameter->get_type(), element[name], dg);
        }
    }
    break;
    case dclass::Type::T_INVALID:
    default:
        assert(false);
        break;
    }
}

class MongoDatabase : public DatabaseBackend
{
  public:
    MongoDatabase(ConfigNode dbeconfig, doid_t min_id, doid_t max_id) :
            DatabaseBackend(dbeconfig, min_id, max_id),
            m_shutdown(false),
            m_monotonic_exhausted(false)
    {
        stringstream log_name;
        log_name << "Database-MongoDB" << "(Range: [" << min_id << ", " << max_id << "])";
        m_log = new LogCategory("mongodb", log_name.str());

        // Init connection.
        string error;
        m_connection_string = ConnectionString::parse(database_address.get_rval(m_config), error);

        if(!m_connection_string.isValid()) {
            m_log->fatal() << "Could not parse connection string: " << error << endl;
            exit(1);
        }

        // Init the collection/database names:
        m_db = m_connection_string.getDatabase(); // NOTE: If this line won't compile, install mongo-cxx-driver's 'legacy' branch.
        m_obj_collection = m_db + ".astron.objects";
        m_global_collection = m_db + ".astron.globals";

        // Init the globals collection/document:
        DBClientBase *client = new_connection();
        BSONObj query = BSON("_id" << "GLOBALS");
        BSONObj globals = BSON("$setOnInsert" << BSON(
                                   "doid" << BSON(
                                       "monotonic" << min_id <<
                                       "free" << BSONArray())
                               ));
        client->update(m_global_collection, query, globals, true);
        delete client;

        // Spawn worker threads:
        for(int i = 0; i < NUM_WORKERS; ++i) {
            m_threads.push_back(new thread(bind(&MongoDatabase::run_thread, this)));
        }
    }

    ~MongoDatabase()
    {
        // Shutdown threads:
        {
            lock_guard<mutex> guard(m_lock);
            m_shutdown = true;
            m_cv.notify_all();
        }
        for(auto it = m_threads.begin(); it != m_threads.end(); ++it) {
            (*it)->join();
            delete *it;
        }

        delete m_log;
    }

    virtual void submit(DBOperation *operation)
    {
        lock_guard<mutex> guard(m_lock);
        m_operation_queue.push(operation);
        m_cv.notify_one();
    }

  private:
    LogCategory *m_log;

    ConnectionString m_connection_string;

    queue<DBOperation *> m_operation_queue;
    condition_variable m_cv;

    mutex m_lock;
    vector<thread *> m_threads;
    bool m_shutdown;

    string m_db;
    string m_obj_collection;
    string m_global_collection;

    // N.B. this variable is NOT guarded by a lock. While there can conceivably
    // be races on accessing it, this is not a problem, because:
    // 1) It is initialized to false by the main thread, and only set to true
    //    by sub-threads. There is no way for this variable to go back from
    //    true to false.
    // 2) It is only used to tell the DOID allocator to stop trying to use the
    //    monotonic counter. If a thread misses the update from false->true,
    //    it will only waste time fruitlessly trying to allocate an ID from
    //    the (exhausted) monotonic counter, before falling back on the free
    //    DOIDs list.
    bool m_monotonic_exhausted;

    DBClientBase *new_connection()
    {
        string error;
        DBClientBase *connection;

        if(!(connection = m_connection_string.connect(error))) {
            m_log->fatal() << "Connection failure: " << error << endl;
            exit(1);
        }

        return connection;
    }

    void run_thread()
    {
        unique_lock<mutex> guard(m_lock);

        DBClientBase *client = new_connection();

        while(true) {
            if(!client->isStillConnected()) {
                m_log->error() << "A thread lost its connection, reconnecting..." << endl;
                delete client;
                client = new_connection();
            }

            if(m_operation_queue.size() > 0) {
                DBOperation *op = m_operation_queue.front();
                m_operation_queue.pop();

                guard.unlock();
                handle_operation(client, op);
                guard.lock();
            } else if(m_shutdown) {
                break;
            } else {
                m_cv.wait(guard);
            }
        }

        delete client;
    }

    void handle_operation(DBClientBase *client, DBOperation *operation)
    {
        // First, figure out what kind of operation it is, and dispatch:
        switch(operation->type()) {
        case DBOperation::OperationType::CREATE_OBJECT: {
            handle_create(client, operation);
        }
        break;
        case DBOperation::OperationType::DELETE_OBJECT: {
            handle_delete(client, operation);
        }
        break;
        case DBOperation::OperationType::GET_OBJECT:
        case DBOperation::OperationType::GET_FIELDS: {
            handle_get(client, operation);
        }
        break;
        case DBOperation::OperationType::SET_FIELDS:
        case DBOperation::OperationType::UPDATE_FIELDS: {
            handle_modify(client, operation);
        }
        break;
        }
    }

    void handle_create(DBClientBase *client, DBOperation *operation)
    {
        // First, let's convert the requested object into BSON; this way, if
        // a failure happens, it happens before we waste a doid.
        BSONObjBuilder fields;

        try {
            for(auto it = operation->set_fields().begin();
                it != operation->set_fields().end();
                ++it) {
                DatagramPtr dg = Datagram::create();
                dg->add_data(it->second);
                DatagramIterator dgi(dg);
                fields << it->first->get_name()
                       << bamboo2bson(it->first->get_type(), dgi)["_"];
            }
        } catch(mongo::DBException &e) {
            m_log->error() << "While formatting "
                           << operation->dclass()->get_name()
                           << " for insertion: " << e.what() << endl;
            operation->on_failure();
            return;
        }

        doid_t doid = assign_doid(client);
        if(doid == INVALID_DO_ID) {
            // The error will already have been emitted at this point, so
            // all that's left for us to do is fail silently:
            operation->on_failure();
            return;
        }

        BSONObj b = BSON("_id" << doid <<
                         "dclass" << operation->dclass()->get_name() <<
                         "fields" << fields.obj());

        m_log->trace() << "Inserting new " << operation->dclass()->get_name()
                       << "(" << doid << "): " << b << endl;

        try {
            client->insert(m_obj_collection, b);
        } catch(mongo::DBException &e) {
            m_log->error() << "Cannot insert new "
                           << operation->dclass()->get_name()
                           << "(" << doid << "): " << e.what() << endl;
            operation->on_failure();
            return;
        }

        operation->on_complete(doid);
    }

    void handle_delete(DBClientBase *client, DBOperation *operation)
    {
        BSONObj result;

        bool success;
        try {
            success = client->runCommand(
                          m_db,
                          BSON("findandmodify" << "astron.objects" <<
                               "query" << BSON(
                                   "_id" << operation->doid()) <<
                               "remove" << true),
                          result);
        } catch(mongo::DBException &e) {
            m_log->error() << "Unexpected error while deleting "
                           << operation->doid() << ": " << e.what() << endl;
            operation->on_failure();
            return;
        }

        m_log->trace() << "handle_delete: got response: "
                       << result << endl;

        // If the findandmodify command failed, there wasn't anything there
        // to delete in the first place.
        if(!success || result["value"].isNull()) {
            m_log->error() << "Tried to delete non-existent doid "
                           << operation->doid() << endl;
            operation->on_failure();
            return;
        }

        free_doid(client, operation->doid());
        operation->on_complete();
    }

    void handle_get(DBClientBase *client, DBOperation *operation)
    {
        BSONObj obj;
        try {
            obj = client->findOne(m_obj_collection,
                                 BSON("_id" << operation->doid()));
        } catch(mongo::DBException &e) {
            m_log->error() << "Unexpected error occurred while trying to"
                           " retrieve object with DOID "
                           << operation->doid() << ": " << e.what() << endl;
            operation->on_failure();
            return;
        }

        if(obj.isEmpty()) {
            m_log->warning() << "Got queried for non-existent object with DOID "
                             << operation->doid() << endl;
            operation->on_failure();
            return;
        }

        DBObjectSnapshot *snap = format_snapshot(operation->doid(), obj);
        if(!snap || !operation->verify_class(snap->m_dclass)) {
            operation->on_failure();
        } else {
            operation->on_complete(snap);
        }
    }

    void handle_modify(DBClientBase *client, DBOperation *operation)
    {
        // First, we have to format our findandmodify.
        BSONObjBuilder sets;
        bool has_sets = false;
        BSONObjBuilder unsets;
        bool has_unsets = false;
        for(auto it  = operation->set_fields().begin();
            it != operation->set_fields().end(); ++it) {
            stringstream fieldname;
            fieldname << "fields." << it->first->get_name();
            if(it->second.empty()) {
                unsets << fieldname.str() << true;
                has_unsets = true;
            } else {
                DatagramPtr dg = Datagram::create();
                dg->add_data(it->second);
                DatagramIterator dgi(dg);
                sets << fieldname.str() << bamboo2bson(it->first->get_type(), dgi)["_"];
                has_sets = true;
            }
        }

        BSONObjBuilder updates_b;
        if(has_sets) {
            updates_b << "$set" << sets.obj();
        }
        if(has_unsets) {
            updates_b << "$unset" << unsets.obj();
        }
        BSONObj updates = updates_b.obj();

        // Also format any criteria for the change:
        BSONObjBuilder query_b;
        query_b << "_id" << operation->doid();
        for(auto it  = operation->criteria_fields().begin();
            it != operation->criteria_fields().end(); ++it) {
            stringstream fieldname;
            fieldname << "fields." << it->first->get_name();
            if(it->second.empty()) {
                query_b << fieldname.str() << BSON("$exists" << false);
            } else {
                DatagramPtr dg = Datagram::create();
                dg->add_data(it->second);
                DatagramIterator dgi(dg);
                query_b << fieldname.str() << bamboo2bson(it->first->get_type(), dgi)["_"];
            }
        }
        BSONObj query = query_b.obj();

        m_log->trace() << "Performing updates to " << operation->doid()
                       << ": " << updates << endl;
        m_log->trace() << "Query is: " << query << endl;

        BSONObj result;
        bool success;
        try {
            success = client->runCommand(
                          m_db,
                          BSON("findandmodify" << "astron.objects"
                               << "query" << query
                               << "update" << updates),
                          result);
        } catch(mongo::DBException &e) {
            m_log->error() << "Unexpected error while modifying "
                           << operation->doid() << ": " << e.what() << endl;
            operation->on_failure();
            return;
        }

        m_log->trace() << "Update result: " << result << endl;

        BSONObj obj;
        if(!success || result["value"].isNull()) {
            // Okay, something didn't work right. If we had criteria, let's
            // try to fetch the object without the criteria to see if it's a
            // criteria mismatch or a missing DOID.
            if(!operation->criteria_fields().empty()) {
                try {
                    obj = client->findOne(m_obj_collection,
                                         BSON("_id" << operation->doid()));
                } catch(mongo::DBException &e) {
                    m_log->error() << "Unexpected error while modifying "
                                   << operation->doid() << ": " << e.what() << endl;
                    operation->on_failure();
                    return;
                }
                if(!obj.isEmpty()) {
                    // There's the problem. Now we can send back a snapshot:
                    DBObjectSnapshot *snap = format_snapshot(operation->doid(), obj);
                    if(snap && operation->verify_class(snap->m_dclass)) {
                        operation->on_criteria_mismatch(snap);
                        return;
                    } else {
                        // Something else weird happened with our snapshot;
                        // either the class wasn't recognized or it was the
                        // wrong class. Either way, an error has been logged,
                        // and we need to fail the operation.
                        operation->on_failure();
                        return;
                    }
                }
            }

            // Nope, not that. We're missing the DOID.
            m_log->error() << "Attempted to modify unknown DOID: "
                           << operation->doid() << endl;
            operation->on_failure();
            return;
        }

        // If we've gotten to this point: Hooray! The change has gone
        // through to the database.
        // Let's, however, double-check our changes. (Specifically, we should
        // run verify_class so that we know the frontend is happy with what
        // kind of object we just modified.)
        obj = result["value"].Obj();
        try {
            string dclass_name = obj["dclass"].String();
            const dclass::Class *dclass = g_dcf->get_class_by_name(dclass_name);
            if(!dclass) {
                m_log->error() << "Encountered unknown database object: "
                               << dclass_name << "(" << operation->doid() << ")" << endl;
            } else if(operation->verify_class(dclass)) {
                // Yep, it all checks out. Complete the operation:
                operation->on_complete();
                return;
            }
        } catch(mongo::DBException &e) { }

        // If we've gotten here, something is seriously wrong. We've just
        // mucked with an object without knowing the consequences! What have
        // we done?! We've created an abomination in the database! Kill it!
        // Kill it with fire!

        // All we really can do to mitigate this is scream at the user (which
        // the above verification has already done by now) and revert the
        // object back to how it was when we found it.
        // NOTE: This DOES have the potential for data loss, because we're
        // wiping out any changes that conceivably could have happened
        // between the findandmodify and now. In dev environments, (which we
        // are probably in right now, if other components are making
        // outlandish requests like this) this shouldn't be a huge issue.
        m_log->trace() << "Reverting changes made to " << operation->doid() << endl;
        try {
            client->update(
                m_obj_collection,
                BSON("_id" << operation->doid()),
                obj);
        } catch(mongo::DBException &e) {
            // Wow, we REALLY fail at life.
            m_log->error() << "Could not revert corrupting changes to "
                           << operation->doid() << ": " << e.what() << endl;
        }
        operation->on_failure();
    }

    // Get a DBObjectSnapshot from a MongoDB BSON object; returns NULL if failure.
    DBObjectSnapshot *format_snapshot(doid_t doid, const BSONObj &obj)
    {
        m_log->trace() << "Formatting database snapshot of " << doid << ": "
                       << obj << endl;
        try {
            string dclass_name = obj["dclass"].String();
            const dclass::Class *dclass = g_dcf->get_class_by_name(dclass_name);
            if(!dclass) {
                m_log->error() << "Encountered unknown database object: "
                               << dclass_name << "(" << doid << ")" << endl;
                return NULL;
            }

            BSONObj fields = obj["fields"].Obj();

            DBObjectSnapshot *snap = new DBObjectSnapshot();
            snap->m_dclass = dclass;
            for(auto it = fields.begin(); it.more(); ++it) {
                const char *name = (*it).fieldName();
                const dclass::Field *field = dclass->get_field_by_name(name);
                if(!field) {
                    m_log->warning() << "Encountered unexpected field " << name
                                     << " while formatting " << dclass_name
                                     << "(" << doid << "); ignored." << endl;
                    continue;
                }
                {
                    DatagramPtr dg = Datagram::create();
                    bson2bamboo(field->get_type(), *it, *dg);
                    snap->m_fields[field].resize(dg->size());
                    memcpy(snap->m_fields[field].data(), dg->get_data(), dg->size());
                }
            }

            return snap;
        } catch(mongo::DBException &e) {
            m_log->error() << "Unexpected error while trying to format"
                           " database snapshot for " << doid << ": "
                           << e.what() << endl;
            return NULL;
        }
    }

    // This function is used by handle_create to get a fresh DOID assignment.
    doid_t assign_doid(DBClientBase *client)
    {
        try {
            if(!m_monotonic_exhausted) {
                doid_t doid = assign_doid_monotonic(client);
                if(doid == INVALID_DO_ID) {
                    m_monotonic_exhausted = true;
                } else {
                    return doid;
                }
            }

            // We've exhausted our supply of doids from the monotonic counter.
            // We must now resort to pulling things out of the free list:
            return assign_doid_reuse(client);
        } catch(mongo::DBException &e) {
            m_log->error() << "Unexpected error occurred while trying to"
                           " allocate a new DOID: " << e.what() << endl;
            return INVALID_DO_ID;
        }
    }

    doid_t assign_doid_monotonic(DBClientBase *client)
    {
        BSONObj result;

        bool success = client->runCommand(
                           m_db,
                           BSON("findandmodify" << "astron.globals" <<
                                "query" << BSON(
                                    "_id" << "GLOBALS" <<
                                    "doid.monotonic" << GTE << m_min_id <<
                                    "doid.monotonic" << LTE << m_max_id
                                ) <<
                                "update" << BSON(
                                    "$inc" << BSON("doid.monotonic" << 1)
                                )), result);

        // If the findandmodify command failed, the document either doesn't
        // exist, or we ran out of monotonic doids.
        if(!success || result["value"].isNull()) {
            return INVALID_DO_ID;
        }

        m_log->trace() << "assign_doid_monotonic: got globals element: "
                       << result << endl;

        doid_t doid;
        const BSONElement &element = result["value"]["doid"]["monotonic"];
        if(sizeof(doid) == sizeof(long long)) {
            doid = element.Long();
        } else if(sizeof(doid) == sizeof(int)) {
            doid = element.Int();
        }
        return doid;
    }

    // This is used when the monotonic counter is exhausted:
    doid_t assign_doid_reuse(DBClientBase *client)
    {
        BSONObj result;

        bool success = client->runCommand(
                           m_db,
                           BSON("findandmodify" << "astron.globals" <<
                                "query" << BSON(
                                    "_id" << "GLOBALS" <<
                                    "doid.free.0" << BSON("$exists" << true)
                                ) <<
                                "update" << BSON(
                                    "$pop" << BSON("doid.free" << -1)
                                )), result);

        // If the findandmodify command failed, the document either doesn't
        // exist, or we ran out of reusable doids.
        if(!success || result["value"].isNull()) {
            m_log->error() << "Could not allocate a reused DOID!" << endl;
            return INVALID_DO_ID;
        }

        m_log->trace() << "assign_doid_reuse: got globals element: "
                       << result << endl;

        // Otherwise, use the first one:
        doid_t doid;
        const BSONElement &element = result["value"]["doid"]["free"];
        if(sizeof(doid) == sizeof(long long)) {
            doid = element.Array()[0].Long();
        } else if(sizeof(doid) == sizeof(int)) {
            doid = element.Array()[0].Int();
        }
        return doid;
    }

    // This returns a DOID to the free list:
    void free_doid(DBClientBase *client, doid_t doid)
    {
        m_log->trace() << "Returning doid " << doid << " to the free pool..." << endl;

        try {
            client->update(
                m_global_collection,
                BSON("_id" << "GLOBALS"),
                BSON("$push" << BSON("doid.free" << doid)));
        } catch(mongo::DBException &e) {
            m_log->error() << "Could not return doid " << doid
                           << " to free pool: " << e.what() << endl;
        }
    }
};

DBBackendFactoryItem<MongoDatabase> mongodb_factory("mongodb");
