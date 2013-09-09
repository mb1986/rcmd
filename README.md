rcmd
====

Remote command invocation tool.

Config proposal
---------------
`` .rcmd.yaml ``
```yaml
global:
    user: "mb"
    host: "192.168.1.202"
    port: 22

command:
    g++:
        cd: "dev/"
    pwd:
        env: {a: "a", b: "b"} # environment variables
```

