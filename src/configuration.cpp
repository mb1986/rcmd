#include "configuration.hpp"

/* constructor */
Configuration::Configuration(std::string const& /*path*/) {
    // TODO --- implement
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

