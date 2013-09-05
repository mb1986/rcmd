#include "ssh_executor.hpp"
#include "configuration.hpp"
#include "command_builder.hpp"

#include <string>
#include <iostream>

int main(int argc, char const* argv[]) {

    CommandBuilder cmd(argc, argv);
    Configuration conf("./.rcmd.yaml"); // FIXME --- cmd name

    SshExecutor ssh(conf, true);
    ssh.exec(cmd.command());

    return 0;
}

