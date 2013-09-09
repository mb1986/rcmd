#ifndef RCMD_COMMAND_BUILDER_HPP
#define RCMD_COMMAND_BUILDER_HPP

#include "configuration.hpp"

#include <string>

class CommandBuilder {
    public:
        CommandBuilder() = delete;
        CommandBuilder(int /*argc*/, char const** /*argv*/, Configuration&);

    public:
        std::string const& command_name() const;
        std::string const& command() const;

    private:
        std::string m_command_name;
        std::string m_command;
};

#endif//RCMD_COMMAND_BUILDER_HPP

