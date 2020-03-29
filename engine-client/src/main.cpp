
#include <routing/fmt.h>

#include <engine.grpc.pb.h>
#include <memory>
#include <type_traits>
#include "engine.pb.h"
#include "grpcpp/create_channel.h"
#include "grpcpp/impl/codegen/channel_interface.h"
#include "grpcpp/impl/codegen/client_context.h"
#include "grpcpp/security/credentials.h"

class EngineClient final
{
public:
    EngineClient(std::shared_ptr<::grpc::Channel> channel)
        : m_stub(engine::EngineService::NewStub(channel))
    {
    }

    void hello(std::string const& request_message)
    {
        engine::HelloRequest request;
        request.set_name(request_message);

        grpc::ClientContext context;

        engine::HelloReply response;

        auto status = m_stub->SayHello(&context, request, &response);

        if (!status.ok())
        {
            fmt::print("failed {}", status.error_message());
            return;
        }

        fmt::print("request {}\n", response.message());
    }

private:
    std::unique_ptr<engine::EngineService::Stub> m_stub;
};

int
main(int argc, char** argv)
{
    fmt::print("Hello world client\n");

    EngineClient client(grpc::CreateChannel(
        "localhost:50051", grpc::InsecureChannelCredentials()));

    client.hello("AAA");

    return 0;
}
