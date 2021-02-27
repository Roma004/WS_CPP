# Simple Chat on C++ and websockets

### Structure of the project:
`main.cpp`         -- a single file wiht all the source code

`test_client.html` -- Minimal testing frontend created using html and js

`uSockets.a`       -- file needed for dynamical linking with uWebSockets - the main dependency of the project.

`Makefile`         -- Linux building projects instructions

### Dependences:

(uWebSockets)[https://github.com/uNetworking/uWebSockets]

(JSON for modern C++)[https://github.com/nlohmann/json]

### Building on Linux:

After installing all the dependences (into the standart include path /usr/include or /usr/local/include):
```bash
make  # build project and run
make build  # only build project
make run  # run already built project
```

### Windows users:

I'm sure, if you are using that OS, you will find a solution, how to run this code ;)
