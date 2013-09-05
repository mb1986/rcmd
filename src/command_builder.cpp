#include "command_builder.hpp"

#include <sstream>
#include <stdexcept>

/* constructor */
CommandBuilder::CommandBuilder(int argc, char const** argv) : m_argc(argc), m_argv(argv) {
}

/* public */
std::string CommandBuilder::command() const {
    if (m_argc < 1) {
        throw std::runtime_error("argc can not be less than 1!");
    }

    int argi = 1;
    std::string cmd;
    std::string arg0 = m_argv[0];
    std::string::size_type dash_pos = arg0.find("-");
    if (dash_pos != std::string::npos) {
        if (arg0.substr(0, dash_pos) == "rcmd") {
            cmd = arg0.substr(dash_pos+1);
        }
    } else {
        if (m_argc >= 2) {
            cmd = m_argv[1];
            ++argi;
        }
    }

    if (cmd.empty()) {
        throw std::runtime_error("Can not find command name!");
    }

    std::stringstream cmd_line;
    cmd_line << cmd;
    for (; argi < m_argc; ++argi) {
        cmd_line << " " << m_argv[argi];
    }
    return cmd_line.str();
}

