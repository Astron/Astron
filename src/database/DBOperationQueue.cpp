#include "DBOperationQueue.h"

DBOperation *DBOperationQueue::get_next_operation()
{
    if(!m_queue.size()) {
        return nullptr;
    }

    DBOperation *op = m_queue.front();

    if(can_operation_start(op)) {
        m_running_operations.insert(op);
        m_queue.pop();
        return op;
    } else {
        return nullptr;
    }
}

bool DBOperationQueue::begin_operation(DBOperation *op)
{
    if(can_operation_start(op)) {
        m_running_operations.insert(op);

        return true;
    } else {
        m_queue.push(op);

        return false;
    }
}

bool DBOperationQueue::finalize_operation(const DBOperation *op)
{
    m_running_operations.erase(op);

    // TODO: This should check to see if it conflicts with the next queued
    // operation. If this didn't conflict, then the next operation in the queue
    // is obviously blocked by something else, and we shouldn't bother to run
    // it through can_operation_start.
    return true;
}

bool DBOperationQueue::can_operation_start(const DBOperation *op)
{
    // TODO: Currently, this just waits until the running operations map is
    // empty. The desired behavior is to check if "op" conflicts with any of the
    // database operations in m_running_operations.
    return !m_running_operations.size();
}
