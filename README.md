rcmd
====

Remote command invocation tool.

Usage
-----
Let us assume that we want to invoke `ls -l` command on the remote machine. Using `rcmd` one may do that in two different, but similar, ways:

1. Type in local machine command prompt `rcmd ls -l` and hit enter.
2. Change the name of `rcmd` executable file to `rcmd-ls` (pay attention to the _dash_) and after that type `rcmd-ls -l` (remember about the _dash_) and hit enter.

It should be noted that the second method may be useful in some cases, e.g. if some other application needs to invoke some command remotely.

In most cases configuration file **`.rcmd.yaml`** will be needed (described in the [Configuration](#configuration) section).

Configuration
-------------
Configuration file, named **`.rcmd.yaml`**, may be located in the local machine _working directory_ of executed command.
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

All of the above options can occur in both sections (`global` and `command` followed by _command name_).

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

