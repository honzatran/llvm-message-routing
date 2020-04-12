
#include <routing/config.h>
#include <routing/fmt.h>
#include <routing/logger.h>

#include <engine.grpc.pb.h>
#include <grpc_options.h>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <type_traits>

#include "absl/types/span.h"
#include "engine.pb.h"
#include "grpcpp/create_channel.h"
#include "grpcpp/impl/codegen/channel_interface.h"
#include "grpcpp/impl/codegen/client_context.h"
#include "grpcpp/security/credentials.h"
#include "routing/file_util.h"
#include "spdlog/fmt/bundled/ostream.h"

class EngineClient final
{
public:
    EngineClient(std::shared_ptr<::grpc::Channel> channel)
        : m_stub(engine::EngineService::NewStub(channel))
    {
        m_logger = routing::get_default_logger("EngineClient");
    }

    void store(std::string const& name, absl::Span<std::uint8_t> data)
    {
        engine::TemplateRegisterRequest request;

        request.set_name(name);
        request.set_source_package(
            reinterpret_cast<void*>(data.begin()), data.size());

        engine::TemplateRegisterReply reply;

        auto status = m_stub->RegisterTemplate(&m_context, request, &reply);

        status.error_code();

        if (!status.ok())
        {
            m_logger->error("Request failed due to {}", status.error_message());
        }
    }

private:
    grpc::ClientContext m_context;
    std::unique_ptr<engine::EngineService::Stub> m_stub;

    routing::Logger_t m_logger;
};

program_options::options_description
get_engine_client_options()
{
    program_options::options_description engine_options;

    engine_options.add_options()(
        "engine.package.path", routing::create_option<std::string>());

    return engine_options;
}

std::vector<program_options::options_description>
get_all_subcomponent_desc()
{
    return {
        get_engine_client_options(),
        routing::get_logger_options_description(),
        routing::grpc::get_grpc_options_description("engine", 50051)};
}

int
main(int argc, char** argv)
{
    fmt::print("Hello world client\n");

    routing::Config config = routing::parse_args(
        get_all_subcomponent_desc(),
        argc,
        argv,
        "../resources/engine-client.properties");

    routing::init_logger(config);

    auto grpc_option = routing::grpc::get_grpc_options("engine", config);

    EngineClient client(grpc::CreateChannel(
        grpc_option.channel, grpc::InsecureChannelCredentials()));

    auto template_path = config.get<std::string>("engine.package.path");

    if (!template_path)
    {
        fmt::print(std::cerr, "No template path specified\n");
        return 1;
    }

    std::filesystem::path path(*template_path);

    if (std::filesystem::is_directory(path))
    {
        auto data = routing::compress_directory(path);

        auto logger = routing::get_default_logger("Main");

        logger->info("{} encoded into {} of bytes", path, data.size());
        client.store("AAA", absl::MakeSpan(data));
    }


    return 0;
}
