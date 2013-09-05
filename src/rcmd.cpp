#include "ssh_executor.hpp"
#include "configuration.hpp"

#include <string>
#include <iostream>

int main(int argc, char const* argv[]) {

    Configuration conf("./.rcmd.yaml");

    SshExecutor ssh(conf, true);
    ssh.exec(argv[1]);

    return 0;
}

