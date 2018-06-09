#pragma once

#include <queue>
#include <mutex>
#include <thread>
#include "core/global.h"
#include "deps/uvw/uvw.hpp"

typedef std::function<void()> TaskCallback;

class TaskQueue
{
    private:
        std::mutex m_queue_mutex;
        std::queue<TaskCallback> m_task_queue;
        std::shared_ptr<uvw::AsyncHandle> m_flush_handle;
        bool m_in_flush = false;
    public:
        ~TaskQueue();
        static TaskQueue singleton;
        void init_queue();
        void enqueue_task(TaskCallback task);
        void flush_tasks();
};
