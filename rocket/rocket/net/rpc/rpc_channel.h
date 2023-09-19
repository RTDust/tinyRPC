#ifndef ROCKET_NET_RPC_RPC_CHANNEL_H
#define ROCKET_NET_RPC_RPC_CHANNEL_H

#include <google/protobuf/service.h>
#include <memory>
#include "rocket/net/tcp/net_addr.h"
#include "rocket/net/tcp/tcp_client.h"
#include "rocket/net/timer_event.h"

namespace rocket
{

#define NEWMESSAGE(type, var_name) \
  std::shared_ptr<type> var_name = std::make_shared<type>();

#define NEWRPCCONTROLLER(var_name) \
  std::shared_ptr<rocket::RpcController> var_name = std::make_shared<rocket::RpcController>();

#define NEWRPCCHANNEL(addr, var_name) \
  std::shared_ptr<rocket::RpcChannel> var_name = std::make_shared<rocket::RpcChannel>(rocket::RpcChannel::FindAddr(addr));

#define CALLRPRC(addr, stub_name, method_name, controller, request, response, closure)                    \
  {                                                                                                       \
    NEWRPCCHANNEL(addr, channel);                                                                         \
    channel->Init(controller, request, response, closure);                                                \
    stub_name(channel.get()).method_name(controller.get(), request.get(), response.get(), closure.get()); \
  }

  /// @brief 针对客户端，封装TCP通信,发包收包等动作
  class RpcChannel : public google::protobuf::RpcChannel, public std::enable_shared_from_this<RpcChannel>
  {

  public:
    typedef std::shared_ptr<RpcChannel> s_ptr;
    typedef std::shared_ptr<google::protobuf::RpcController> controller_s_ptr;
    typedef std::shared_ptr<google::protobuf::Message> message_s_ptr;
    typedef std::shared_ptr<google::protobuf::Closure> closure_s_ptr;

  public:
    // 获取 addr
    // 若 str 是 ip:port, 直接返回
    // 否则认为是 rpc 服务名，尝试从配置文件里面获取对应的 ip:port（后期会加上服务发现）
    static NetAddr::s_ptr FindAddr(const std::string &str);

  public:
    RpcChannel(NetAddr::s_ptr peer_addr);

    ~RpcChannel();

    void Init(controller_s_ptr controller, message_s_ptr req, message_s_ptr res, closure_s_ptr done);

    void CallMethod(const google::protobuf::MethodDescriptor *method,
                    google::protobuf::RpcController *controller, const google::protobuf::Message *request,
                    google::protobuf::Message *response, google::protobuf::Closure *done);

    google::protobuf::RpcController *getController();

    google::protobuf::Message *getRequest();

    google::protobuf::Message *getResponse();

    google::protobuf::Closure *getClosure();

    TcpClient *getTcpClient();

  private:
    void callBack();

  private:
    NetAddr::s_ptr m_peer_addr{nullptr};
    NetAddr::s_ptr m_local_addr{nullptr};

    controller_s_ptr m_controller{nullptr};
    message_s_ptr m_request{nullptr};
    message_s_ptr m_response{nullptr};
    closure_s_ptr m_closure{nullptr};

    /// @brief 是否初始化成功
    bool m_is_init{false};

    TcpClient::s_ptr m_client{nullptr};
  };

}

#endif