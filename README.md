rcmd
====

Remote command invocation tool.

Configuration
-------------
Configuration file, named **`.rcmd.yaml`**, may be located in the _working directory_ of executed command.
Syntax is imposed by the **YAML** format.

File is divided into two main sections:
* `global` - section conataining options for all commands,
* `command` - section for options for particular commands.

#### Supported options
* `user` - user name,
* `host` - host name,
* `port` - port number,
* `cd` - directory to change to,
* `env` - set environment variables (remember about `AcceptEnv`)
* `map` - map command to the new value,
* `verbose` - switches on verbosity.

#### Configuration file example
```yaml
global:
    user: "mb"
    host: "192.168.1.202"
    port: 22
    verbose: true

command:
    make:
        cd: "dev/project"
        env:
            CXX: "g++"
            CXX_FLAGS: "-std=c++11"
    dir:
        map: "ls -l"
        verbose: false
```

