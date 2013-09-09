#include "configuration.hpp"

/* constructor */
Configuration::Configuration(std::string const& path) {
    try {
        m_config = YAML::LoadFile(path);
        YAML::Node const& config_global = m_config["global"];
        if (config_global) {
            parse_config(config_global);
        }
    } catch (YAML::BadFile const&) {
        // do nothing
    } catch (YAML::Exception const& ex) {
        throw std::runtime_error("Configuration file error ('" + ex.msg + "')!");
    }
}

/* public */
void Configuration::command(std::string const& cmd_name) {
    YAML::Node const& config_command = m_config["command"];
    try {
        if (config_command) {
            YAML::Node const& config_command_node = config_command[cmd_name];
            if (config_command_node) {
                parse_config(config_command_node);
            }
        }
    } catch (YAML::Exception const& ex) {
        throw std::runtime_error("Configuration file error ('" + ex.msg + "')!");
    }
}

/* public */
std::string const& Configuration::username() const {
    return m_username;
}

/* public */
std::string const& Configuration::hostname() const {
    return m_hostname;
}

/* public */
uint16_t Configuration::port() const {
    return m_port;
}

/* public */
std::string const& Configuration::directory() const {
    return m_directory;
}

/* public */
std::string const& Configuration::command_mapping() const {
    return m_map;
}

/* public */
Configuration::env_vars_type const& Configuration::environment_vars() const {
    return m_env_vars;
}

/* public */
bool Configuration::verbosity() const {
    return m_verbosity;
}

/* private */
void Configuration::parse_config(YAML::Node const& node) {
    if (node["user"]) {
        m_username = node["user"].as<std::string>();
    }
    if (node["host"]) {
        m_hostname = node["host"].as<std::string>();
    }
    if (node["port"]) {
        m_port = node["port"].as<uint16_t>();
    }
    if (node["cd"]) {
        m_directory = node["cd"].as<std::string>();
    }
    if (node["verbose"]) {
        m_verbosity = node["verbose"].as<bool>();
    }
    if (node["env"]) {
        for (auto const& it : node["env"]) {
            m_env_vars.emplace_back(it.first.as<std::string>(), it.second.as<std::string>());
        }
    }
    if (node["map"]) {
        m_map = node["map"].as<std::string>();
    }
}

