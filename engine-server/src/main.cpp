
#include <routing/config.h>
#include <routing/fatal_signal_handlers.h>
#include <routing/fmt.h>
#include <routing/logger.h>

#include <engine.grpc.pb.h>
#include <grpc_options.h>

#include <grpc++/server_builder.h>
#include "engine.pb.h"
#include "grpcpp/impl/codegen/server_context.h"
#include "grpcpp/impl/codegen/status.h"
#include "grpcpp/security/server_credentials.h"
#include "routing/file_util.h"
#include "routing/logger.h"

#include <cstdint>
#include <filesystem>

class Engine final : public engine::EngineService::Service
{
public:
    Engine(std::filesystem::path package_path);

    ::grpc::Status RegisterTemplate(
        ::grpc::ServerContext *context,
        ::engine::TemplateRegisterRequest const *request,
        ::engine::TemplateRegisterReply *response) override;

private:
    routing::Logger_t m_logger;

    std::filesystem::path m_package_path;
};

Engine::Engine(std::filesystem::path package_path)
    : m_package_path(package_path)
{
    m_logger = routing::get_default_logger("Engine");

    if (!std::filesystem::is_directory(m_package_path))
    {
        std::filesystem::create_directory(m_package_path);
    }
}

::grpc::Status
Engine::RegisterTemplate(
    ::grpc::ServerContext *context,
    engine::TemplateRegisterRequest const *request,
    engine::TemplateRegisterReply *response)
{
    std::string const &name = request->name();

    std::filesystem::path package_path
        = m_package_path / std::filesystem::path(request->name());

    if (std::filesystem::is_directory(package_path)
        || std::filesystem::create_directory(package_path))
    {
        response->set_result(true);

        std::string package_source = request->source_package();
        absl::Span<std::uint8_t> package_span(
            reinterpret_cast<std::uint8_t *>(&package_source[0]),
            package_source.length());

        routing::decode_directory(package_path, package_span);
    }

    return grpc::Status::OK;
}

program_options::options_description
get_engine_options()
{
    program_options::options_description engine_options;

    engine_options.add_options()(
        "engine.package.path",
        routing::create_option<std::string>("../resources/package-path"));

    return engine_options;
}

std::vector<program_options::options_description>
get_all_subcomponent_desc()
{
    return {
        routing::get_logger_options_description(),
        routing::grpc::get_grpc_options_description("engine", 50051),
        get_engine_options()};
}

int
main(int argc, char **argv)
{
    fmt::print("Hello world!!\n");

    routing::Config config = routing::parse_args(
        get_all_subcomponent_desc(),
        argc,
        argv,
        "../resources/engine-server.properties");

    routing::init_logger(config);

    std::filesystem::path package_path(
        config.get_option<std::string>("engine.package.path"));
    Engine engine(package_path);

    grpc::ServerBuilder server_builder;
    server_builder.RegisterService(&engine);

    auto grpc_options = routing::grpc::get_grpc_options("engine", config);

    server_builder.AddListeningPort(
        grpc_options.channel, grpc::InsecureServerCredentials());

    auto server = server_builder.BuildAndStart();

    server->Wait();
    return 0;
}
