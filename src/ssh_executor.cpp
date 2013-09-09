#include "ssh_executor.hpp"

#include <Shlobj.h>

#include <cstdio>
#include <sstream>
#include <iostream>
#include <stdexcept>

#define sleep(seconds) Sleep((seconds)*1000)

/* constructor */
SshExecutor::SshExecutor(Configuration const& configuration) : m_conf(configuration) {

#ifdef WITH_VERBOSE
        if (m_conf.verbosity()) {
            std::cerr << " --- Ssh hostname set to: '" << m_conf.hostname() << "'." << std::endl;
            std::cerr << " --- Ssh port set to: '" << m_conf.port() << "'." << std::endl;
        }
#endif//WITH_VERBOSE

        WSADATA wsadata;
        ::WSAStartup(MAKEWORD(2,0), &wsadata); // 2,2

        //TODO gethostbyname(hostname.c_str());
        m_hostaddr = ::inet_addr(m_conf.hostname().c_str());
        if (m_hostaddr == INADDR_NONE) {
            throw std::runtime_error("Wrong hostname!");
        }

        m_sockaddr.sin_family = AF_INET;
        m_sockaddr.sin_port = ::htons(m_conf.port());
        m_sockaddr.sin_addr.s_addr = m_hostaddr;

#ifdef WITH_VERBOSE
        if (m_conf.verbosity()) {
            std::cerr << " --- Initializing ssh." << std::endl;
        }
#endif//WITH_VERBOSE
        if (::libssh2_init(0)) {
            throw std::runtime_error("Can not initialize ssh!");
        }
    }

/* destructor */
SshExecutor::~SshExecutor() {
#ifdef WITH_VERBOSE
    if (m_conf.verbosity()) {
        std::cerr << " --- Deinitializing ssh." << std::endl;
    }
#endif//WITH_VERBOSE
    ::libssh2_exit();
    ::WSACleanup();
}

/* public */
bool SshExecutor::exec(std::string const& cmd) {
    if(!tryConnect()) {
        throw std::runtime_error("Could not connect!");
    }

    int rc;

    //::libssh2_channel_handle_extended_data(m_channel, LIBSSH2_CHANNEL_EXTENDED_DATA_NORMAL);

//    while ((rc = ::libssh2_channel_request_pty(m_channel, "vt100")) == LIBSSH2_ERROR_EAGAIN) {
//        wait();
//    }
//    if (rc) {
//#ifdef WITH_VERBOSE
//        if (m_conf.verbosity()) {
//            std::cerr << " !!! Requesting pty failed (e: " << rc << ")." << std::endl;
//        }
//#endif//WITH_VERBOSE
//        disconnect();
//        return false;
//    }

    // set environment variables
//    for (auto const& it : m_conf.environment_vars()) {
//        while ((rc = ::libssh2_channel_setenv(m_channel, it.first.c_str(), it.second.c_str())) == LIBSSH2_ERROR_EAGAIN) {
//            wait();
//        }
//        if (rc) {
//#ifdef WITH_VERBOSE
//            if (m_conf.verbosity()) {
//                std::cerr << " !!! Environment variable setting failed (e: " << rc << ")." << std::endl;
//            }
//#endif//WITH_VERBOSE
//            disconnect();
//            return false;
//        }
//    }

    std::stringstream command;
    if (!m_conf.directory().empty()) {
        command << "cd '" << m_conf.directory() << "' && ";
    }
    command << cmd;

#ifdef WITH_VERBOSE
    if (m_conf.verbosity()) {
        std::cerr << " --- Executing command: '" << cmd << "'." << std::endl;
    }
#endif//WITH_VERBOSE
    while ((rc = ::libssh2_channel_exec(m_channel, command.str().c_str())) == LIBSSH2_ERROR_EAGAIN) {
        wait();
    }
    if (rc) {
#ifdef WITH_VERBOSE
        if (m_conf.verbosity()) {
            std::cerr << " !!! Command execution failed (e: " << rc << ")." << std::endl;
        }
#endif//WITH_VERBOSE
        disconnect();
        return false;
    }


    char buffer[4096];
    int n;
#ifdef WITH_VERBOSE
    if (m_conf.verbosity()) {
        std::cerr << " --- Reading data." << std::endl;
    }
#endif//WITH_VERBOSE
    while (true) {
        while ((n = ::libssh2_channel_read(m_channel, buffer, sizeof(buffer))) == LIBSSH2_ERROR_EAGAIN);

        if (n > 0) {
            std::cout << std::string(buffer, n) << std::flush;
        } else if (n == 0) {
            break;
        } else {
#ifdef WITH_VERBOSE
            if (m_conf.verbosity()) {
                std::cerr << " !!! Reading failed." << std::endl;
            }
#endif//WITH_VERBOSE
            disconnect();
            return false;
        }
    }

#ifdef WITH_VERBOSE
    if (m_conf.verbosity()) {
        std::cerr << " --- Reading error data." << std::endl;
    }
#endif//WITH_VERBOSE
    while (true) {
        while ((n = ::libssh2_channel_read_stderr(m_channel, buffer, sizeof(buffer))) == LIBSSH2_ERROR_EAGAIN);

        if (n > 0) {
            std::cerr << std::string(buffer, n) << std::flush;
        } else if (n == 0) {
            break;
        } else {
#ifdef WITH_VERBOSE
            if (m_conf.verbosity()) {
                std::cerr << " !!! Reading failed." << std::endl;
            }
#endif//WITH_VERBOSE
            disconnect();
            return false;
        }
    }
#ifdef WITH_VERBOSE
    if (m_conf.verbosity()) {
        std::cerr << " --- Reading success." << std::endl;
    }
#endif//WITH_VERBOSE

    disconnect();
    return true;
}

/* public */
void SshExecutor::setRetryLimit(uint8_t limit) {
    m_retry_limit = limit;
}

/* public */
void SshExecutor::setRetrySleep(uint8_t sleep) {
    m_retry_sleep = sleep;
}

/* public */
void SshExecutor::setRetrySleepAdd(uint8_t sleep_add) {
    m_retry_sleep_add = sleep_add;
}

/* private */
bool SshExecutor::connect() {
    int rc;

#ifdef WITH_VERBOSE
    if (m_conf.verbosity()) {
        std::cerr << " --- Opening socket." << std::endl;
    }
#endif//WITH_VERBOSE
    m_socket = ::socket(AF_INET, SOCK_STREAM, 0);
    rc = ::connect(m_socket, (struct sockaddr*)(&m_sockaddr), sizeof(struct sockaddr_in));
    if (rc != 0) {
#ifdef WITH_VERBOSE
        if (m_conf.verbosity()) {
            std::cerr << " !!! Socket connection failed." << std::endl;
        }
#endif//WITH_VERBOSE
        disconnect();
        return false;
    }

#ifdef WITH_VERBOSE
    if (m_conf.verbosity()) {
        std::cerr << " --- Connecting." << std::endl;
    }
#endif//WITH_VERBOSE
    m_session = ::libssh2_session_init();
    if (!m_session) {
#ifdef WITH_VERBOSE
        if (m_conf.verbosity()) {
            std::cerr << " !!! Session initialization failed." << std::endl;
        }
#endif//WITH_VERBOSE
        disconnect();
        return false;
    }
    ::libssh2_session_set_blocking(m_session, 0);

#ifdef WITH_VERBOSE
    if (m_conf.verbosity()) {
        std::cerr << " --- Performing handshake." << std::endl;
    }
#endif//WITH_VERBOSE
    while ((rc = ::libssh2_session_handshake(m_session, m_socket)) == LIBSSH2_ERROR_EAGAIN) {
        wait();
    }
    if (rc) {
#ifdef WITH_VERBOSE
        if (m_conf.verbosity()) {
            std::cerr << " !!! Session handshake failed (e: " << rc << "/" << ::WSAGetLastError() << ")." << std::endl;
        }
#endif//WITH_VERBOSE
        disconnect();
        return false;
    }

    // TODO --- known hosts !!!

    TCHAR homePath[MAX_PATH];
    if (!SUCCEEDED(::SHGetFolderPath(NULL, CSIDL_PROFILE, NULL, 0, homePath))) {
#ifdef WITH_VERBOSE
        if (m_conf.verbosity()) {
            std::cerr << " !!! Retrieving user home directory failed." << std::endl;
        }
#endif//WITH_VERBOSE
        disconnect();
        return false;
    }
    char pubKeyPath[MAX_PATH];
    ::snprintf(pubKeyPath, sizeof(pubKeyPath), "%s%s", homePath, "/.ssh/id_rsa.pub");
    char privKeyPath[MAX_PATH];
    ::snprintf(privKeyPath, sizeof(privKeyPath), "%s%s", homePath, "/.ssh/id_rsa");
#ifdef WITH_VERBOSE
    if (m_conf.verbosity()) {
        std::cerr << " --- Authenticating user '" << m_conf.username() << "' by public key: '" << pubKeyPath << "'." << std::endl;
    }
#endif//WITH_VERBOSE
    while ((rc = ::libssh2_userauth_publickey_fromfile(m_session, m_conf.username().c_str(),
                    pubKeyPath, privKeyPath, "")) == LIBSSH2_ERROR_EAGAIN);
    if (rc) {
#ifdef WITH_VERBOSE
        if (m_conf.verbosity()) {
            std::cerr << " !!! Public key authentication failed (e: " << rc << ")." << std::endl;
        }
#endif//WITH_VERBOSE
        disconnect();
        return false;
    }

#ifdef WITH_VERBOSE
    if (m_conf.verbosity()) {
        std::cerr << " --- Opening channel." << std::endl;
    }
#endif//WITH_VERBOSE
    while((m_channel = ::libssh2_channel_open_session(m_session)) == nullptr &&
           ::libssh2_session_last_error(m_session, nullptr, nullptr, 0) == LIBSSH2_ERROR_EAGAIN) {
        wait();
    }
    if(m_channel == nullptr) {
#ifdef WITH_VERBOSE
        if (m_conf.verbosity()) {
            std::cerr << " !!! Chanel open failed." << std::endl;
        }
#endif//WITH_VERBOSE
        disconnect();
        return false;
    }

    return true;
}

/* private */
bool SshExecutor::tryConnect() {
    int retry_limit = m_retry_limit;
    int retry_sleep = m_retry_sleep;
    bool connected;
    while (!(connected = connect()) && (retry_limit-- > 0)) {
#ifdef WITH_VERBOSE
        if (m_conf.verbosity()) {
            std::cerr << " === Retrying connection attempt." << std::endl;
        }
#endif//WITH_VERBOSE
        ::sleep(retry_sleep);
        retry_sleep += m_retry_sleep_add;
    }
    return connected;
}

/* private */
void SshExecutor::disconnect() {
    if (m_channel) {
#ifdef WITH_VERBOSE
        if (m_conf.verbosity()) {
            std::cerr << " --- Closing channel." << std::endl;
        }
#endif//WITH_VERBOSE
        while(::libssh2_channel_close(m_channel) == LIBSSH2_ERROR_EAGAIN) {
            wait();
        }
#ifdef WITH_VERBOSE
        if (m_conf.verbosity()) {
            std::cerr << " --- Waiting for remote to close channel." << std::endl;
        }
#endif//WITH_VERBOSE
        while(::libssh2_channel_wait_closed(m_channel) == LIBSSH2_ERROR_EAGAIN) {
            wait();
        }
        m_channel = nullptr;
    }

    if (m_session) {
#ifdef WITH_VERBOSE
        if (m_conf.verbosity()) {
            std::cerr << " --- Disconnecting." << std::endl;
        }
#endif//WITH_VERBOSE
        ::libssh2_session_disconnect(m_session, "OK");
        ::libssh2_session_free(m_session);
        m_session = nullptr;
    }

    if (m_socket != INVALID_SOCKET) {
#ifdef WITH_VERBOSE
        if (m_conf.verbosity()) {
            std::cerr << " --- Closing socket." << std::endl;
        }
#endif//WITH_VERBOSE
        ::closesocket(m_socket);
        m_socket = INVALID_SOCKET;
    }
}

/* private */
int SshExecutor::wait() {
    if (m_socket == INVALID_SOCKET || m_session == nullptr) {
#ifdef WITH_VERBOSE
        if (m_conf.verbosity()) {
            std::cerr << " !!! Can not wait." << std::endl;
        }
#endif//WITH_VERBOSE
        return -1;
    }

    fd_set fd;
    fd_set* writefd = nullptr;
    fd_set* readfd = nullptr;

    FD_ZERO(&fd);
    FD_SET(m_socket, &fd);

    int dir = libssh2_session_block_directions(m_session);
    if(dir & LIBSSH2_SESSION_BLOCK_INBOUND) {
        readfd = &fd;
    }
    if(dir & LIBSSH2_SESSION_BLOCK_OUTBOUND) {
        writefd = &fd;
    }

#ifdef WITH_VERBOSE
    if (m_conf.verbosity()) {
        std::cerr << " --- Waiting." << std::endl;
    }
#endif//WITH_VERBOSE
    return select(m_socket + 1, readfd, writefd, nullptr, &m_wait_timeout);
}

