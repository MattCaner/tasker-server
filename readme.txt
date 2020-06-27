**USER MANUAL**

2nd project for Programming in Unix Systems, WFiIS AGH, 2020

Author: Maciej Trzciński

**COMPILATION AND FILES**

Please use provided makefile to get sender.out, server.out and receiver.out files.
If you wish to test the program, run 'test.sh' or 'make test'. This however requires gnome-launcher to work properly, so may not be available on all the possible unix machines.

test.out is a simple executable that counts prime numbers, it is provided for example and test reasons.

**INSTALLATION AND RUNNING**

Correct order of running executables:
->first, run server.out somewhere within the reach of your network.
->second, run receiver.out on machines you want to listen for updates.
->third, run sender.out, providing a task to be performed as an argument (more below).

You can run more listeners while the server is running. Listeners will automatically try to connect with the server they have in config as soon as they are started.

In case of running more than one of receivers and/or both receivers and the server on the same machine (sharing the same address) they NEED to run at different ports.

Should a server try connect to a receiver on its list but if fails, it starts counting failure connections to this particular receiver. In case of consecutive failures exceeding allowed amount, the receiver will be removed from the list.

**EXAMPLE CONFIG FILES**

Example config files, server_config.conf, sender_config.conf, receiver_config.conf are provided as an example of how config files can be created.

**SERVER**

Usage: ./server.out [config_file]
Config file requires the following structure:

port=<port to run this program on, (e.g. 7777)>
max_connection_failures=<integer number (e.g. 4)>
max_listeners=<integer number (e.g. 50)>

'port' needs to be the very first string in the file. Every line, including the last one, has to be ended with newline. If the config is incorrect in any way, the behaviour is undefined. If config file is not supplied, default values will be chosen instead.

If you want receiver to be removed from the list as soon as they are unreachable for the first time, set max_connection_failures to 1.

**SENDER**

Usage: /.sender.out <config_file> <program_to_run> [any number of arguments of executed program]
Config file requires the following structure:

server=<address of the server (e.g. 127.0.0.1)>
server_port=<port the server runs at (e.g. 7777)>
custom_msg=<a message that will be appended to message sent by this sender (e.g. sender_one)>

'server' needs to be the very first string in the file. Every line, including the last one, has to be ended with newline. If the config is incorrect in any way, the behaviour is undefined.
The message should be maximum 128 charactes, no whitespaces.

**RECEIVER**

Usage: /.receiver.out <config_file>
Config file requires the following structure:

port=<own port, at which the receiver will listen (e.g. 7788)>
server=<address of the server (e.g. 192.168.0.22)>
server_port=<port the server runs at (e.g. 7777)>

'port' needs to be the very first string in the file. Every line, including the last one, has to be ended with newline. If the config is incorrect in any way, the behaviour is undefined.
