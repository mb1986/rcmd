#include "ssh_executor.hpp"

#include <iostream>
#include <stdexcept>

#define sleep(seconds) Sleep((seconds)*1000)

/* constructor */
SshExecutor::SshExecutor(std::string const& hostname, uint16_t port, bool verbose) :
    m_hostname(hostname),
    m_port(port),
    m_verbose(verbose) {

#ifndef NDEBUG
        if (m_verbose) {
            std::cerr << " --- Ssh hostname set to: '" << hostname << "'." << std::endl;
            std::cerr << " --- Ssh port set to: '" << port << "'." << std::endl;
        }
#endif//NDEBUG

        WSADATA wsadata;
        WSAStartup(MAKEWORD(2,0), &wsadata); // 2,2

        m_hostaddr = ::inet_addr(hostname.c_str());
        if (m_hostaddr == INADDR_NONE) {
            throw std::runtime_error("Wrong hostname!");
        }

        m_sockaddr.sin_family = AF_INET;
        m_sockaddr.sin_port = ::htons(m_port);
        m_sockaddr.sin_addr.s_addr = m_hostaddr;

#ifndef NDEBUG
        if (m_verbose) {
            std::cerr << " --- Initializing ssh." << std::endl;
        }
#endif//NDEBUG
        if (::libssh2_init(0)) {
            throw std::runtime_error("Can not initialize ssh!");
        }
    }

/* destructor */
SshExecutor::~SshExecutor() {
#ifndef NDEBUG
    if (m_verbose) {
        std::cerr << " --- Deinitializing ssh." << std::endl;
    }
#endif//NDEBUG
    ::libssh2_exit();
}

/* public */
bool SshExecutor::exec(std::string const& cmd, std::string const& path) {
    if(!tryConnect()) {
        throw std::runtime_error("Could not connect!");
    }

    // if (path.empty())

    disconnect();
    return false;
}

/* private */
bool SshExecutor::connect() {
    int rc;

#ifndef NDEBUG
    if (m_verbose) {
        std::cerr << " --- Opening socket." << std::endl;
    }
#endif//NDEBUG
    m_socket = ::socket(AF_INET, SOCK_STREAM, 0);
    rc = ::connect(m_socket, (struct sockaddr*)(&m_sockaddr), sizeof(struct sockaddr_in));
    if (rc != 0) {
#ifndef NDEBUG
        if (m_verbose) {
            std::cerr << " !!! Socket connection failed." << std::endl;
        }
#endif//NDEBUG
        disconnect();
        return false;
    }

#ifndef NDEBUG
    if (m_verbose) {
        std::cerr << " --- Connecting." << std::endl;
    }
#endif//NDEBUG
    m_session = ::libssh2_session_init();
    if (!m_session) {
#ifndef NDEBUG
        if (m_verbose) {
            std::cerr << " !!! Session initialization failed." << std::endl;
        }
#endif//NDEBUG
        disconnect();
        return false;
    }
    ::libssh2_session_set_blocking(m_session, 0);

#ifndef NDEBUG
    if (m_verbose) {
        std::cerr << " --- Performing handshake." << std::endl;
    }
#endif//NDEBUG
    while ((rc = ::libssh2_session_handshake(m_session, m_socket)) == LIBSSH2_ERROR_EAGAIN);
    if (rc != 0) {
#ifndef NDEBUG
        if (m_verbose) {
            std::cerr << " !!! Session handshake failed." << std::endl;
        }
#endif//NDEBUG
        disconnect();
        return false;
    }

    return true;
}

/* private */
bool SshExecutor::tryConnect() {
    int retry_no = m_retry_no;
    int retry_sleep = m_retry_sleep;
    bool connected;
    while (!(connected = connect()) && (retry_no-- > 0)) {
#ifndef NDEBUG
        if (m_verbose) {
            std::cerr << " === Retrying connection attempt." << std::endl;
        }
#endif//NDEBUG
        ::sleep(retry_sleep);
        retry_sleep += m_retry_sleep_add;
    }
    return connected;
}

/* private */
void SshExecutor::disconnect() {
    if (m_session) {
#ifndef NDEBUG
        if (m_verbose) {
            std::cerr << " --- Disconnecting." << std::endl;
        }
#endif//NDEBUG
        ::libssh2_session_disconnect(m_session, "OK");
        ::libssh2_session_free(m_session);
        m_session = nullptr;
    }

    if (m_socket != INVALID_SOCKET) {
#ifndef NDEBUG
        if (m_verbose) {
            std::cerr << " --- Closing socket." << std::endl;
        }
#endif//NDEBUG
        closesocket(m_socket);
        m_socket = INVALID_SOCKET;
    }
}

