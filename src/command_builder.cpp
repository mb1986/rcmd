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
        std::stringstream command_args;
        command_args << " ";
        for (; argi < argc; ++argi) {
            command_args << " " << argv[argi];
        }
        m_command_args = command_args.str();
    }
    makeCommand();
}

/* public */
std::string const& CommandBuilder::command_name() const {
    return m_command_name;
}

/* public */
std::string const& CommandBuilder::command() const {
    return m_command;
}

/* public */
void CommandBuilder::map_command_name(std::string const& new_command_name) {
    if (!new_command_name.empty()) {
        m_command_name = new_command_name;
        makeCommand();
    }
}

/* private */
void CommandBuilder::makeCommand() {
    m_command = m_command_name + m_command_args;
}

