#ifndef ROCKET_NET_EVENTLOOP_H
#define ROCKET_NET_EVENTLOOP_H

#include <pthread.h>
#include <set>
#include <functional>
#include <queue>
#include "rocket/common/mutex.h"
#include "rocket/net/fd_event.h"
#include "rocket/net/wakeup_fd_event.h"
#include "rocket/net/timer.h"

namespace rocket
{
  /// @brief 时间循环类——通过该类调用EventLoop循环，是对其进行的再封装
  class EventLoop
  {
  public:
    EventLoop();

    ~EventLoop();

    void loop();

    void wakeup();

    void stop();

    void addEpollEvent(FdEvent *event);

    void deleteEpollEvent(FdEvent *event);

    bool isInLoopThread();

    void addTask(std::function<void()> cb, bool is_wake_up = false);

    void addTimerEvent(TimerEvent::s_ptr event);

    bool isLooping();

  public:
    static EventLoop *GetCurrentEventLoop();

  private:
    void dealWakeup();

    void initWakeUpFdEevent();

    void initTimer();

  private:
    pid_t m_thread_id{0};

    int m_epoll_fd{0}; // epoll句柄，会占用一个fd值。因此在使用完epoll后，必须调用close()关闭，否则有可能导致fd被耗尽

    int m_wakeup_fd{0}; // wakeup fd，用来唤醒epoll_wait

    WakeUpFdEvent *m_wakeup_fd_event{NULL};

    bool m_stop_flag{false};

    /// @brief 用set容器保存监听套接字事件
    std::set<int> m_listen_fds;

    /// @brief 回调函数队列——负责处理相应fd的事件——即所有的待执行任务队列
    std::queue<std::function<void()>> m_pending_tasks;
    Mutex m_mutex; // 互斥锁

    Timer *m_timer{NULL};

    bool m_is_looping{false};
  };

}

#endif