#include "proxy.h"
#include <netdb.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h> 
#include <err.h>

int main(int argc, char* argv[])
{
    int port;
    if (argc != 3) {
        err(EXIT_FAILURE, "usage: %s [ip] [port]", argv[0]);
    }  

    port = strtol(argv[2], NULL, 10);

    set_name("P1");
    set_ip_port(argv[1], port);
    notify_ready_shutdown();
    while(get_clock_lamport() != 5);
    notify_shutdown_ack();
}
