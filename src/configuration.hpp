#ifndef RCMD_CONFIGURATION_HPP
#define RCMD_CONFIGURATION_HPP

#include <yaml-cpp/yaml.h>

#include <vector>
#include <utility>
#include <string>

class Configuration {
    using env_vars_type = std::vector<std::pair<std::string, std::string>>;

    public:
        Configuration() = delete;
        Configuration(std::string const& /*path*/, std::string const& /*cmd_name*/);

    public:
        std::string const& username() const;
        std::string const& hostname() const;
        uint16_t port() const;
        std::string const& directory() const;
        env_vars_type const& environment_vars() const;

        bool verbosity() const;

    private:
        void parse_config(YAML::Node const&);

    private:
        std::string m_username = "guest"; // FIXME --- logged user name
        std::string m_hostname = "127.0.0.1"; // FIXME --- "localhost"
        uint16_t m_port = 22;
        std::string m_directory;

        env_vars_type m_env_vars;

        bool m_verbosity = false;
};

#endif//RCMD_CONFIGURATION_HPP

