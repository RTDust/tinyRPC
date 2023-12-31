
#include "rocket/net/rpc/rpc_controller.h"

namespace rocket
{

  /// RPC调用过程中配置文件

  /// @brief 重置RPC调用的控制器，使其能够再次调用RPC服务
  void RpcController::Reset()
  {
    m_error_code = 0;
    m_error_info = "";
    m_msg_id = "";
    m_is_failed = false;
    m_is_cancled = false;
    m_is_finished = false;
    m_local_addr = nullptr;
    m_peer_addr = nullptr;
    m_timeout = 1000; // ms
  }

  /// @brief 返回RPC调用是否失败
  /// @return 失败则返回true，成功则返回false
  bool RpcController::Failed() const
  {
    return m_is_failed;
  }

  /// @brief 获取调用RPC失败的错误信息
  /// @return 返回具体的错误文本
  std::string RpcController::ErrorText() const
  {
    return m_error_info;
  }

  void RpcController::StartCancel()
  {
    m_is_cancled = true;
    m_is_failed = true;
    SetFinished(true);
  }

  void RpcController::SetFailed(const std::string &reason)
  {
    m_error_info = reason;
    m_is_failed = true;
  }

  bool RpcController::IsCanceled() const
  {
    return m_is_cancled;
  }

  void RpcController::NotifyOnCancel(google::protobuf::Closure *callback)
  {
  }

  void RpcController::SetError(int32_t error_code, const std::string error_info)
  {
    m_error_code = error_code;
    m_error_info = error_info;
    m_is_failed = true;
  }

  int32_t RpcController::GetErrorCode()
  {
    return m_error_code;
  }

  std::string RpcController::GetErrorInfo()
  {
    return m_error_info;
  }

  /// @brief 设置RPC调用的ID
  /// @param  msg_id RPC调用ID
  void RpcController::SetMsgId(const std::string &msg_id)
  {
    m_msg_id = msg_id;
  }

  /// @brief 获取RPC id
  /// @return 
  std::string RpcController::GetMsgId()
  {
    return m_msg_id;
  }

  void RpcController::SetLocalAddr(NetAddr::s_ptr addr)
  {
    m_local_addr = addr;
  }

  void RpcController::SetPeerAddr(NetAddr::s_ptr addr)
  {
    m_peer_addr = addr;
  }

  NetAddr::s_ptr RpcController::GetLocalAddr()
  {
    return m_local_addr;
  }

  NetAddr::s_ptr RpcController::GetPeerAddr()
  {
    return m_peer_addr;
  }

  /// @brief 设置超时时间
  /// @param timeout 超时时间设置
  void RpcController::SetTimeout(int timeout)
  {
    m_timeout = timeout;
  }

  int RpcController::GetTimeout()
  {
    return m_timeout;
  }

  bool RpcController::Finished()
  {
    return m_is_finished;
  }

  void RpcController::SetFinished(bool value)
  {
    m_is_finished = value;
  }

}