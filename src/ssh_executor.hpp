#ifndef RCMD_SSH_EXECUTOR_HPP
#define RCMD_SSH_EXECUTOR_HPP

#include "configuration.hpp"

#include <libssh2.h>
#include <winsock2.h>

#include <string>

class SshExecutor {
    public:
        SshExecutor(Configuration const& /*configuration*/);
        ~SshExecutor();

        bool exec(std::string const& /*cmd*/);

        void setRetryLimit(uint8_t /*limit*/);
        void setRetrySleep(uint8_t /*sleep*/);
        void setRetrySleepAdd(uint8_t /*sleep_add*/);

    private:
        bool connect();
        bool tryConnect();
        void disconnect();
        int wait();

    private:
        std::string m_username;
        std::string m_hostname;
        unsigned long m_hostaddr;
        uint16_t m_port;
        std::string m_directory;
        struct sockaddr_in m_sockaddr;
        unsigned int m_socket = INVALID_SOCKET;

        struct timeval m_wait_timeout = {10, 0};

        bool m_verbose;
        uint8_t m_retry_limit = 3;
        uint8_t m_retry_sleep = 1;
        uint8_t m_retry_sleep_add = 1;

        LIBSSH2_SESSION* m_session = nullptr;
        LIBSSH2_CHANNEL* m_channel = nullptr;
};

#endif//RCMD_SSH_EXECUTOR_HPP

