#ifndef RCMD_COMMAND_BUILDER_HPP
#define RCMD_COMMAND_BUILDER_HPP

#include <string>

class CommandBuilder {
    public:
        CommandBuilder() = delete;
        CommandBuilder(int /*argc*/, char const** /*argv*/);

    public:
        std::string const& command() const;

    private:
        int m_argc;
        char const** m_argv;

};

#endif//RCMD_COMMAND_BUILDER_HPP

