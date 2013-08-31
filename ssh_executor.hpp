#ifndef RCMD_SSH_EXECUTOR_HPP
#define RCMD_SSH_EXECUTOR_HPP

#include <libssh2.h>
#include <winsock2.h>

#include <string>

class SshExecutor {
    public:
        SshExecutor(std::string const& /*hostname*/, uint16_t /*port*/, bool /*verbose*/ = false);
        ~SshExecutor();

        bool exec(std::string const& /*cmd*/, std::string const& /*path*/ = std::string());

    private:
        bool connect();
        bool tryConnect();
        void disconnect();

    private:
        std::string m_hostname;
        unsigned long m_hostaddr;
        uint16_t m_port;
        struct sockaddr_in m_sockaddr;
        unsigned int m_socket = INVALID_SOCKET;

        bool m_verbose;
        int m_retry_no = 3;
        int m_retry_sleep = 1;
        int m_retry_sleep_add = 1;


        LIBSSH2_SESSION* m_session = nullptr;
        LIBSSH2_CHANNEL* m_channel = nullptr;
};

#endif//RCMD_SSH_EXECUTOR_HPP

