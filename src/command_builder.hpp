#ifndef RCMD_COMMAND_BUILDER_HPP
#define RCMD_COMMAND_BUILDER_HPP

#include <string>

class CommandBuilder {
    public:
        CommandBuilder() = delete;
        CommandBuilder(int /*argc*/, char const** /*argv*/);

    public:
        std::string const& command_name() const;
        std::string const& command() const;

        void map_command_name(std::string const& /*new_command_name*/);

    private:
        void makeCommand();

    private:
        std::string m_command_name;
        std::string m_command_args;
        std::string m_command;
};

#endif//RCMD_COMMAND_BUILDER_HPP

