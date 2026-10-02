#include "ConfigLoader.hpp"
#include <toml++/toml.h>

Config ConfigLoader::load(const std::string& filename) {
    auto table = toml::parse_file(filename);
    Config config;

    config.listen_port = table["listen_port"].value_or(0);
    config.algorithm = table["algorithm"].value_or("");

    if (auto backends = table["backends"].as_array())
    {
        for (auto&& backend : *backends)
        {
            if (auto backend_table = backend.as_table())
            {
                BackendConfig backend_config;

                backend_config.host =
                    (*backend_table)["host"].value_or("");

                backend_config.port =
                    (*backend_table)["port"].value_or(0);

                backend_config.weight =
                    (*backend_table)["weight"].value_or(0);

                config.backends.push_back(backend_config);
            }
        }
    }

    return config;
}