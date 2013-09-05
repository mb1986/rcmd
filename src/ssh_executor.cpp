#include "ssh_executor.hpp"

#include <Shlobj.h>

#include <cstdio>
#include <sstream>
#include <iostream>
#include <stdexcept>

#define sleep(seconds) Sleep((seconds)*1000)

/* constructor */
SshExecutor::SshExecutor(Configuration const& configuration, bool verbose) :
    m_username(configuration.username()),
    m_hostname(configuration.hostname()),
    m_port(configuration.port()),
    m_verbose(verbose) {

#ifdef WITH_VERBOSE
        if (m_verbose) {
            std::cerr << " --- Ssh hostname set to: '" << m_hostname << "'." << std::endl;
            std::cerr << " --- Ssh port set to: '" << m_port << "'." << std::endl;
        }
#endif//WITH_VERBOSE

        WSADATA wsadata;
        ::WSAStartup(MAKEWORD(2,0), &wsadata); // 2,2

        //TODO gethostbyname(hostname.c_str());
        m_hostaddr = ::inet_addr(m_hostname.c_str());
        if (m_hostaddr == INADDR_NONE) {
            throw std::runtime_error("Wrong hostname!");
        }

        m_sockaddr.sin_family = AF_INET;
        m_sockaddr.sin_port = ::htons(m_port);
        m_sockaddr.sin_addr.s_addr = m_hostaddr;

#ifdef WITH_VERBOSE
        if (m_verbose) {
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
    if (m_verbose) {
        std::cerr << " --- Deinitializing ssh." << std::endl;
    }
#endif//WITH_VERBOSE
    ::libssh2_exit();
    ::WSACleanup();
}

/* public */
bool SshExecutor::exec(std::string const& cmd, std::string const& path) {
    if(!tryConnect()) {
        throw std::runtime_error("Could not connect!");
    }

    std::stringstream command;
    if (!path.empty()) {
        command << "cd " << path << " && ";
    }
    command << cmd;

#ifdef WITH_VERBOSE
    if (m_verbose) {
        std::cerr << " --- Executing command: '" << cmd << "'." << std::endl;
    }
#endif//WITH_VERBOSE
    int rc;
    while((rc = ::libssh2_channel_exec(m_channel, command.str().c_str())) == LIBSSH2_ERROR_EAGAIN) {
        wait();
    }
    if (rc) {
#ifdef WITH_VERBOSE
        if (m_verbose) {
            std::cerr << " !!! Command execution failed (e: " << rc << ")." << std::endl;
        }
#endif//WITH_VERBOSE
        disconnect();
        return false;
    }


    char buffer[4096];
    int n;
#ifdef WITH_VERBOSE
    if (m_verbose) {
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
            if (m_verbose) {
                std::cerr << " !!! Reading failed." << std::endl;
            }
#endif//WITH_VERBOSE
            disconnect();
            return false;
        }
    }

#ifdef WITH_VERBOSE
    if (m_verbose) {
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
            if (m_verbose) {
                std::cerr << " !!! Reading failed." << std::endl;
            }
#endif//WITH_VERBOSE
            disconnect();
            return false;
        }
    }
#ifdef WITH_VERBOSE
    if (m_verbose) {
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
    if (m_verbose) {
        std::cerr << " --- Opening socket." << std::endl;
    }
#endif//WITH_VERBOSE
    m_socket = ::socket(AF_INET, SOCK_STREAM, 0);
    rc = ::connect(m_socket, (struct sockaddr*)(&m_sockaddr), sizeof(struct sockaddr_in));
    if (rc != 0) {
#ifdef WITH_VERBOSE
        if (m_verbose) {
            std::cerr << " !!! Socket connection failed." << std::endl;
        }
#endif//WITH_VERBOSE
        disconnect();
        return false;
    }

#ifdef WITH_VERBOSE
    if (m_verbose) {
        std::cerr << " --- Connecting." << std::endl;
    }
#endif//WITH_VERBOSE
    m_session = ::libssh2_session_init();
    if (!m_session) {
#ifdef WITH_VERBOSE
        if (m_verbose) {
            std::cerr << " !!! Session initialization failed." << std::endl;
        }
#endif//WITH_VERBOSE
        disconnect();
        return false;
    }
    ::libssh2_session_set_blocking(m_session, 0);

#ifdef WITH_VERBOSE
    if (m_verbose) {
        std::cerr << " --- Performing handshake." << std::endl;
    }
#endif//WITH_VERBOSE
    while ((rc = ::libssh2_session_handshake(m_session, m_socket)) == LIBSSH2_ERROR_EAGAIN) {
        wait();
    }
    if (rc) {
#ifdef WITH_VERBOSE
        if (m_verbose) {
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
        if (m_verbose) {
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
    if (m_verbose) {
        std::cerr << " --- Authenticating user '" << m_username << "' by public key: '" << pubKeyPath << "'." << std::endl;
    }
#endif//WITH_VERBOSE
    while ((rc = ::libssh2_userauth_publickey_fromfile(m_session, m_username.c_str(),
                    pubKeyPath, privKeyPath, "")) == LIBSSH2_ERROR_EAGAIN);
    if (rc) {
#ifdef WITH_VERBOSE
        if (m_verbose) {
            std::cerr << " !!! Public key authentication failed (e: " << rc << ")." << std::endl;
        }
#endif//WITH_VERBOSE
        disconnect();
        return false;
    }

#ifdef WITH_VERBOSE
    if (m_verbose) {
        std::cerr << " --- Opening channel." << std::endl;
    }
#endif//WITH_VERBOSE
    while((m_channel = ::libssh2_channel_open_session(m_session)) == nullptr &&
           ::libssh2_session_last_error(m_session, nullptr, nullptr, 0) == LIBSSH2_ERROR_EAGAIN) {
        wait();
    }
    if(m_channel == nullptr) {
#ifdef WITH_VERBOSE
        if (m_verbose) {
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
        if (m_verbose) {
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
        if (m_verbose) {
            std::cerr << " --- Closing channel." << std::endl;
        }
#endif//WITH_VERBOSE
        while(::libssh2_channel_close(m_channel) == LIBSSH2_ERROR_EAGAIN) {
            wait();
        }
#ifdef WITH_VERBOSE
        if (m_verbose) {
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
        if (m_verbose) {
            std::cerr << " --- Disconnecting." << std::endl;
        }
#endif//WITH_VERBOSE
        ::libssh2_session_disconnect(m_session, "OK");
        ::libssh2_session_free(m_session);
        m_session = nullptr;
    }

    if (m_socket != INVALID_SOCKET) {
#ifdef WITH_VERBOSE
        if (m_verbose) {
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
        if (m_verbose) {
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
    if (m_verbose) {
        std::cerr << " --- Waiting." << std::endl;
    }
#endif//WITH_VERBOSE
    return select(m_socket + 1, readfd, writefd, nullptr, &m_wait_timeout);
}

