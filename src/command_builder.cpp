#include "command_builder.hpp"

/* constructor */
CommandBuilder::CommandBuilder(int argc, char const** argv) : m_argc(argc), m_argv(argv) {
}

/* public */
std::string const& CommandBuilder::command() const {
    return "pwd"; // TODO --- implement
}

