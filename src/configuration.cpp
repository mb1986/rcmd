#include "configuration.hpp"

/* constructor */
Configuration::Configuration(std::string const& path, std::string const& cmd_name) {
    try {
        YAML::Node config = YAML::LoadFile(path);
        YAML::Node const& config_global = config["global"];
        YAML::Node const& config_command = config["command"];

        if (config_global) {
            parse_config(config_global);
        }

        if (config_command) {
            YAML::Node const& config_command_node = config_command[cmd_name];
            if (config_command_node) {
                parse_config(config_command_node);
            }
        }

    } catch (YAML::BadFile const&) {
        // do nothing
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
}

