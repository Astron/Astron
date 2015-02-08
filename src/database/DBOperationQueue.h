#pragma once
#include <queue>
#include <unordered_set>

#include "DBOperation.h"

// Represents the database operations queued up for a single object, blocking any
// operations that may conflict with operations currently being run. This is
// to ensure that any given sequence of operations will complete as if they had
// run one at a time.
class DBOperationQueue
{
  public:
    // Return the next operation that we can run on this object, or nullptr if
    // there are no safe operations in the queue.
    // This will automatically mark the returned operation as "running"
    DBOperation *get_next_operation();

    // Inform the queue that we'd like to begin an operation. If the operation
    // can start right away, this returns false, If this cannot start, it will
    // be enqueued and this function will return true.
    bool enqueue_operation(DBOperation *op);

    // Inform the queue that an operation is now running.
    void begin_operation(DBOperation *op);

    // Inform the queue that an operation has completed. If this might allow the
    // next queued operation to begin, this will return true.
    bool finalize_operation(const DBOperation *op);

    // Is this queue empty (i.e. can it safely be deleted from the map)?
    bool is_empty() const;

  private:
    std::queue<DBOperation *> m_queue;
    std::unordered_set<const DBOperation *> m_running_operations;

    // Inspects an operation to see if it can begin immediately.
    bool can_operation_start(const DBOperation *op);
};
