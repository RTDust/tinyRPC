
#ifndef ROCKET_NET_FDEVENT_H
#define ROCKET_NET_FDEVENT_H

#include <functional>
#include <sys/epoll.h>

namespace rocket
{
  class FdEvent
  {
  public:
    enum TriggerEvent
    {
      IN_EVENT = EPOLLIN,     // 读事件
      OUT_EVENT = EPOLLOUT,   // 写事件
      ERROR_EVENT = EPOLLERR, // 错误事件
    };

    FdEvent(int fd);

    FdEvent();

    ~FdEvent();

    void setNonBlock();

    std::function<void()> handler(TriggerEvent event_type);

    /// @brief 监听触发事件，传入回调函数
    /// @param event_type 触发事件类型
    /// @param callback 回调函数
    /// @param error_callback 错误回调函数
    void listen(TriggerEvent event_type, std::function<void()> callback, std::function<void()> error_callback = nullptr);

    // 取消监听
    void cancle(TriggerEvent event_type);

    int getFd() const
    {
      return m_fd;
    }

    epoll_event getEpollEvent()
    {
      return m_listen_events;
    }

  protected:
    int m_fd{-1};

    epoll_event m_listen_events;

    std::function<void()> m_read_callback{nullptr};  // 读回调函数
    std::function<void()> m_write_callback{nullptr}; // 写回调函数
    std::function<void()> m_error_callback{nullptr}; // 错误回调函数
  };

}

#endif