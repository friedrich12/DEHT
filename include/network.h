#pragma once

#include <string>
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>


void read_from_socket(int socket, unsigned int x, char* buffer);

void send_to_socket(int socket, std::string msg);
