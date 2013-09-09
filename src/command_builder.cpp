#include "command_builder.hpp"

#include <sstream>
#include <stdexcept>

/* constructor */
CommandBuilder::CommandBuilder(int argc, char const** argv) {
    if (argc < 1) {
        throw std::runtime_error("argc can not be less than 1!");
    }

    int argi = 1;
    {
        std::string cmd_name;
        std::string arg0 = argv[0];
        std::string::size_type dash_pos = arg0.find("-");
        if (dash_pos != std::string::npos) {
            if (arg0.substr(0, dash_pos) == "rcmd") {
                cmd_name = arg0.substr(dash_pos+1);
            }
        } else {
            if (argc >= 2) {
                cmd_name = argv[1];
                ++argi;
            }
        }

        if (cmd_name.empty()) {
            throw std::runtime_error("Can not find command name!");
        }
        m_command_name = cmd_name;
    }

    {
        std::stringstream command;
        command << m_command_name;
        for (; argi < argc; ++argi) {
            command << " " << argv[argi];
        }
        m_command = command.str();
    }
}

/* public */
std::string const& CommandBuilder::command_name() const {
    return m_command_name;
}

/* public */
std::string const& CommandBuilder::command() const {
    return m_command;
}

