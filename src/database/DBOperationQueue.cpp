#include "DBOperationQueue.h"

#include <algorithm>

using namespace std;

DBOperation *DBOperationQueue::get_next_operation()
{
    if(!m_queue.size()) {
        return nullptr;
    }

    DBOperation *op = m_queue.front();

    if(can_operation_start(op)) {
        m_queue.pop();
        return op;
    } else {
        return nullptr;
    }
}

bool DBOperationQueue::enqueue_operation(DBOperation *op)
{
    if(!m_queue.size() && can_operation_start(op)) {
        // The queue is empty and this operation can go straight away! Let's
        // refuse to enqueue it.
        return false;
    } else {
        m_queue.push(op);

        return true;
    }
}

void DBOperationQueue::begin_operation(DBOperation *op)
{
    m_running_operations.insert(op);
}

bool DBOperationQueue::finalize_operation(const DBOperation *op)
{
    m_running_operations.erase(op);

    // We return true if there's a chance that this finalize could allow more
    // operations to begin. The way we detect this is by seeing if the
    // newly-finished operation is independent of the next operation in the
    // queue. If they are independent, we can conclude that the front of the
    // queue isn't waiting because of this operation, and there's no sense in
    // trying to flush the queue. In other words, the only time we consider the
    // next operation potentially viable is if something that was blocking it
    // previously is no longer blocking it now.
    return m_queue.size() && !op->is_independent_of(m_queue.front());
}

bool DBOperationQueue::is_empty() const
{
    return !m_queue.size() && !m_running_operations.size();
}

bool DBOperationQueue::can_operation_start(const DBOperation *op)
{
    return all_of(m_running_operations.begin(), m_running_operations.end(),
    [op](const DBOperation * running) {
        return op->is_independent_of(running);
    });
}
