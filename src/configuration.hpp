#ifndef RCMD_CONFIGURATION_HPP
#define RCMD_CONFIGURATION_HPP

#include <string>

class Configuration {
    public:
        Configuration() = delete;
        Configuration(std::string const& /*path*/);

    public:
        std::string const& username() const;
        std::string const& hostname() const;
        uint16_t port() const;

    private:
        std::string m_username = "guest"; // FIXME --- logged user name
        std::string m_hostname = "127.0.0.1"; // FIXME --- "localhost"
        uint16_t m_port = 22;
};

#endif//RCMD_CONFIGURATION_HPP

