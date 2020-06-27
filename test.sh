#!/bin/bash

gnome-terminal -- bash -c "./server.out server_config.conf; exec bash"
sleep 1s
gnome-terminal -- bash -c "./receiver.out receiver_config.conf; exec bash"
sleep 1s
gnome-terminal -- bash -c "./sender.out sender_config.conf test.out; exec bash"
