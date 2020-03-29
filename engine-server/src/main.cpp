
#include <routing/fmt.h>

#include <engine.grpc.pb.h>

#include <grpc++/server_builder.h>
#include "grpcpp/security/server_credentials.h"

class Engine final : public engine::EngineService::Service
{
public:
    ::grpc::Status SayHello(
        ::grpc::ServerContext *context,
        ::engine::HelloRequest const *request,
        ::engine::HelloReply *response) override
    {
        response->set_message("HELLO");
        fmt::print("Received a request with message {}\n", request->name());
        return grpc::Status::OK;
    }

private:
};

int
main(int argc, char **argv)
{
    fmt::print("Hello world!!\n");

    std::string server_address("0.0.0.0:50051");

    Engine engine;

    grpc::ServerBuilder server_builder;
    server_builder.RegisterService(&engine);

    server_builder.AddListeningPort(
        server_address, grpc::InsecureServerCredentials());

    auto server = server_builder.BuildAndStart();

    server->Wait();
    return 0;
}
