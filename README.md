# Simple Chat on C++ and websockets

### Structure of the project:
`main.cpp`         -- a single file wiht all the source code
`test_client.html` -- Minimal testing frontend created using html and js
`uSockets.a`       -- file needed for dynamical linking (https://github.com/uNetworking/uWebSockets) for more details (only for Linux users, If you use Windows, you have to install all the environment for yourself)
`Makefile`         -- Linux building projects instructions

### Dependences:

(https://github.com/uNetworking/uWebSockets) [uWebSockets]

(https://github.com/nlohmann/json) [JSON for modern C++]

### Building on Linux:

After installing all the dependences (into the standart include path /usr/include or /usr/local/include):
```bash
make  # build project and run
make build  # only build project
make run  # run already built project
```

### Windows users:

I'm sure, if you are using that OS, you will find a solution, how to run this code ;)