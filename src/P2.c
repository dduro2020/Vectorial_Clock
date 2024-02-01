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
#include <time.h>

#define P1 0
#define P3 1

int main(int argc, char* argv[]) /*Necesario conectar primero P1 y más tarde P3*/
{                
    int port;
    if (argc != 3) {
        err(EXIT_FAILURE, "usage: %s [ip] [port]", argv[0]);
    }  

    port = strtol(argv[2], NULL, 10);

    set_name("P2");
    set_on_proxy_server(argv[1], port);
    listen_message(2);
    save_sockets();
    while(get_clock_lamport() != 3);
    notify_shutdown_now(P1);
    listen_message(P1);
    while(get_clock_lamport() != 7);
    notify_shutdown_now(P3);
    listen_message(P3);
    while(get_clock_lamport() != 11);
    sleep(2);
    printf("Los clientes fueron correctamente apagados en t(lamport) = %d\n", get_clock_lamport());
} 