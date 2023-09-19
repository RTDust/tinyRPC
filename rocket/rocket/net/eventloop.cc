#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <string.h>
#include "rocket/net/eventloop.h"
#include "rocket/common/log.h"
#include "rocket/common/util.h"

// 通过宏定义定义的模块，方便后期调用
#define ADD_TO_EPOLL()                                                                    \
  auto it = m_listen_fds.find(event->getFd());                                            \
  int op = EPOLL_CTL_ADD;                                                                 \
  if (it != m_listen_fds.end())                                                           \
  {                                                                                       \
    op = EPOLL_CTL_MOD;                                                                   \
  }                                                                                       \
  epoll_event tmp = event->getEpollEvent();                                               \
  INFOLOG("epoll_event.events = %d", (int)tmp.events);                                    \
  int rt = epoll_ctl(m_epoll_fd, op, event->getFd(), &tmp);                               \
  if (rt == -1)                                                                           \
  {                                                                                       \
    ERRORLOG("failed epoll_ctl when add fd, errno=%d, error=%s", errno, strerror(errno)); \
  }                                                                                       \
  m_listen_fds.insert(event->getFd()); /*将当前eventfd加入监听套接字集合中*/ \
  DEBUGLOG("add event success, fd[%d]", event->getFd())

#define DELETE_TO_EPOLL()                                                                   \
  auto it = m_listen_fds.find(event->getFd());                                              \
  if (it == m_listen_fds.end())                                                             \
  {                                                                                         \
    return;                                                                                 \
  }                                                                                         \
  int op = EPOLL_CTL_DEL;                                                                   \
  epoll_event tmp = event->getEpollEvent();                                                 \
  int rt = epoll_ctl(m_epoll_fd, op, event->getFd(), NULL);                                 \
  \ 
  if (rt == -1)                                                                             \
  {                                                                                         \
    ERRORLOG("failed epoll_ctl when add fd, errno=%d, error=%s", errno, strerror(errno));   \
  }                                                                                         \
  m_listen_fds.erase(event->getFd()); /*将当前eventfd从监听套接字集合中删除*/ \
  DEBUGLOG("delete event success, fd[%d]", event->getFd());
/*epoll_ctl是epoll的时间注册函数，epoll_ctl向epoll对象中添加、修改或删除感兴趣的事件，0表示成功，-1表示失败*/

namespace rocket
{

  static thread_local EventLoop *t_current_eventloop = NULL; // 线程局部变量，它表示当前线程的EventLoop
  static int g_epoll_max_timeout = 10000;                    // 最大超时时间，避免长时间陷入epoll_wait中
  static int g_epoll_max_events = 10;                        // 最大监听事件数量

  EventLoop::EventLoop()
  {
    if (t_current_eventloop != NULL)
    {
      ERRORLOG("failed to create event loop, this thread has created event loop");
      exit(0); // 退出程序
    }
    m_thread_id = getThreadId(); // 将该具体的对象同当前的线程id绑定

    m_epoll_fd = epoll_create(10); // 创建一个epoll的句柄，size用来告诉内核这个监听的数目一共有多大。这个数已经没什么用了，给一个比零大的值即可

    if (m_epoll_fd == -1)
    {
      ERRORLOG("failed to create event loop, epoll_create error, error info[%d]", errno);
      exit(0);
    }

    initWakeUpFdEevent();
    initTimer();

    INFOLOG("succ create event loop in thread %d", m_thread_id);
    t_current_eventloop = this; // 把当前对象的指针给它
  }

  EventLoop::~EventLoop()
  {
    close(m_epoll_fd); // 关闭epoll监听
    if (m_wakeup_fd_event)
    {
      delete m_wakeup_fd_event;
      m_wakeup_fd_event = NULL;
    }
    if (m_timer)
    {
      delete m_timer;
      m_timer = NULL;
    }
  }

  void EventLoop::initTimer()
  {
    m_timer = new Timer();
    addEpollEvent(m_timer);
  }

  void EventLoop::addTimerEvent(TimerEvent::s_ptr event)
  {
    m_timer->addTimerEvent(event);
  }

  void EventLoop::initWakeUpFdEevent()
  {

    m_wakeup_fd = eventfd(0, EFD_NONBLOCK); // 事件fd类型，即专门用于事件通知的文件描述符（fd）。设置为非阻塞

    if (m_wakeup_fd < 0)
    {
      ERRORLOG("failed to create event loop, eventfd create error, error info[%d]", errno);
      exit(0);
    }
    INFOLOG("wakeup fd = %d", m_wakeup_fd);

    m_wakeup_fd_event = new WakeUpFdEvent(m_wakeup_fd);

    m_wakeup_fd_event->listen(FdEvent::IN_EVENT, [this]()
                              {
                              char buf[8];
                              while(read(m_wakeup_fd, buf, 8) != -1 && errno != EAGAIN) {
                              }
    DEBUGLOG("read full bytes from wakeup fd[%d]", m_wakeup_fd); });

    addEpollEvent(m_wakeup_fd_event);
  }

  void EventLoop::loop()
  {
    m_is_looping = true; // 打开循环标志位
    while (!m_stop_flag)
    {
      std::queue<std::function<void()>> tmp_tasks; // 回调函数，即相关请求任务队列
      ScopeMutex<Mutex> lock(m_mutex);             // 为什么要加锁？因为任务队列可能会被其他队伍耽误
      m_pending_tasks.swap(tmp_tasks);             // 把都有的任务取出来
      lock.unlock();

      while (!tmp_tasks.empty())
      {
        std::function<void()> cb = tmp_tasks.front(); // 取出任务队列中的第一个任务
        tmp_tasks.pop();
        if (cb)
        {
          cb(); // 执行任务队列中相应的回调函数
        }
      }

      // 如果有定时任务需要执行，那么执行
      // 1. 怎么判断一个定时任务需要执行？ （now() > TimerEvent.arrtive_time）
      // 2. arrtive_time 如何让 eventloop 监听

      int timeout = g_epoll_max_timeout;             // epoll_wait的超时时间
      epoll_event result_events[g_epoll_max_events]; // epoll_event数组
      // epoll_wait(epoll_create创建的句柄, epoll_event*的指针, 当前需要监听的所有socket句柄数, epoll_wait的超时时间)
      int rt = epoll_wait(m_epoll_fd, result_events, g_epoll_max_events, timeout); // 怎么从epoll_wait函数中唤醒

      if (rt < 0) // 返回值小于零，则代表epoll_wait出现问题
      {
        ERRORLOG("epoll_wait error, errno=%d, error=%s", errno, strerror(errno));
      }
      else
      {
        for (int i = 0; i < rt; ++i)
        {
          epoll_event trigger_event = result_events[i];
          FdEvent *fd_event = static_cast<FdEvent *>(trigger_event.data.ptr);
          if (fd_event == NULL)
          {
            ERRORLOG("fd_event = NULL, continue");
            continue;
          }

          // int event = (int)(trigger_event.events);
          // DEBUGLOG("unkonow event = %d", event);

          if (trigger_event.events & EPOLLIN) // 只读事件
          {

            // DEBUGLOG("fd %d trigger EPOLLIN event", fd_event->getFd())
            addTask(fd_event->handler(FdEvent::IN_EVENT));
          }
          if (trigger_event.events & EPOLLOUT) // 只写事件
          {
            // DEBUGLOG("fd %d trigger EPOLLOUT event", fd_event->getFd())
            addTask(fd_event->handler(FdEvent::OUT_EVENT));
          }

          // EPOLLHUP EPOLLERR
          if (trigger_event.events & EPOLLERR) // 错误事件
          {
            DEBUGLOG("fd %d trigger EPOLLERROR event", fd_event->getFd())
            // 删除出错的套接字
            deleteEpollEvent(fd_event);
            if (fd_event->handler(FdEvent::ERROR_EVENT) != nullptr)
            {
              DEBUGLOG("fd %d add error callback", fd_event->getFd())
              addTask(fd_event->handler(FdEvent::OUT_EVENT));
            }
          }
        }
      }
    }
  }

  void EventLoop::wakeup()
  {
    INFOLOG("WAKE UP");
    m_wakeup_fd_event->wakeup();
  }

  void EventLoop::stop()
  {
    m_stop_flag = true;
    wakeup();
  }

  // 处理唤醒事件，唤醒时间只是唤醒线程，并不进行实际的业务操作，因此该函数为空
  void EventLoop::dealWakeup()
  {
  }

  void EventLoop::addEpollEvent(FdEvent *event)
  {
    if (isInLoopThread()) // 判断当前对象绑定的线程是否和当前线程是同一个线程，是，则直接加入epoll事件
    {
      ADD_TO_EPOLL();
    }
    else
    {
      /*如果当前*/
      auto cb = [this, event]()
      {
        ADD_TO_EPOLL();
      };
      addTask(cb, true);
    }
  }

  void EventLoop::deleteEpollEvent(FdEvent *event)
  {
    if (isInLoopThread())
    {
      DELETE_TO_EPOLL();
    }
    else
    {

      auto cb = [this, event]()
      {
        DELETE_TO_EPOLL();
      };
      addTask(cb, true);
    }
  }

  // 将当前回调函数加入任务队列执行
  void EventLoop::addTask(std::function<void()> cb, bool is_wake_up /*=false*/)
  {
    ScopeMutex<Mutex> lock(m_mutex);
    m_pending_tasks.push(cb);
    lock.unlock();

    if (is_wake_up)
    {
      wakeup();
    }
  }

  /// @brief 判断当前对象绑定的线程是否和当前线程是同一个线程
  /// @return 是则返回true，否则返回false
  bool EventLoop::isInLoopThread()
  {
    return getThreadId() == m_thread_id;
  }

  EventLoop *EventLoop::GetCurrentEventLoop()
  {
    if (t_current_eventloop)
    {
      return t_current_eventloop;
    }
    t_current_eventloop = new EventLoop();
    return t_current_eventloop;
  }

  bool EventLoop::isLooping()
  {
    return m_is_looping;
  }

}