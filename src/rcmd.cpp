#include "ssh_executor.hpp"
#include "configuration.hpp"
#include "command_builder.hpp"

#include <string>
#include <iostream>

int main(int argc, char const* argv[]) {

    Configuration conf("./.rcmd.yaml");
    CommandBuilder cmd(argc, argv);
    conf.command_name(cmd.command_name());
    cmd.map_command_name(conf.map());

    SshExecutor ssh(conf);
    ssh.exec(cmd.command());

    return 0;
}

