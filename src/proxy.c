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
#include <pthread.h>
#include <sys/select.h>

#define BACKLOG 10
#define PROCESSES 2

int connect_socket;    /*auxiliar socket for implementation*/
int l_clock;           /*lamport clock of running proccess*/
char name_p[3];        /*name of running proccess*/
int connfd[PROCESSES]; /*client sockets, P1 ALWAYS IN 0, P3 ALWAYS IN 1*/
int auxfd[PROCESSES];
int servfd;            /*server socket*/

/* Establece el nombre del proceso (para los logs y trazas) */
void set_name (char name[2])
{
    strncpy(name_p, name, 3);
}

/* Imprime por pantalla la operacion correspondiente a cada numero */
void print_operation(int n)
{
    char operation[20];
    if (n == 0) {
        strncpy(operation, "READY_TO_SHUTDOWN", 18);
    } else if (n == 1) {
        strncpy(operation, "SHUTDOWN_NOW", 13);
    } else {
        strncpy(operation, "SHUTDOWN_ACK", 13);
    }
    printf(" %s\n", operation);
}

/* Envia un mensaje a quien se le pida */
void send_message(int action) {
    struct message message;
    message.clock_lamport = l_clock + 1;
    strncpy(message.origin, name_p, 3);
    message.action = action;

    if (send(connect_socket, &message, sizeof(message), 0) == -1) {
        err(EXIT_FAILURE, "error: failure sending message\n");
    }
    /*print message*/
    printf("%s, %d, SEND,", name_p, message.clock_lamport); 
    print_operation(message.action);
}

/* Funcion que crea los hilos de escucha*/
void listen_message(int n)
{
    int i;
    pthread_t thread[PROCESSES]; /*max threads at the same time*/
    if (n == 2) {
        for (i = 0; i < PROCESSES; i++){
            /*threads creation*/
            if (pthread_create(&thread[i], NULL, handle_connection, &connfd[i]) != 0) {
                err(EXIT_FAILURE, "error: cannot create thread\n");
            }
        }
        for (i = 0; i < PROCESSES; i++){
            if (pthread_join(thread[i], NULL) != 0) {
                err(EXIT_FAILURE, "error: cannot join thread\n");
            }
        }
    } else {
        if (pthread_create(&thread[n], NULL, handle_connection, &connfd[n]) != 0) {
            err(EXIT_FAILURE, "error: cannot create thread\n");
        }
    }
}

void save_sockets() {
    connfd[0] = auxfd[0];
    connfd[1] = auxfd[1];
}

/* Implementacion de los hilos */
void * handle_connection(void *p_client_socket) {
    int *client_socket = p_client_socket;
    struct message message_in;
        
    if (recv(*client_socket, &message_in, sizeof(message_in), 0) == -1) {
        err(EXIT_FAILURE, "error: failure receiving message\n");
    }

    if (strcmp(message_in.origin, "P1") == 0) {
        auxfd[0] = *client_socket;    
    
    } else if (strcmp(message_in.origin, "P3") == 0) {
        auxfd[1] = *client_socket;
    }

    if (strcmp(message_in.origin, "\0") != 0) {
        if (l_clock < message_in.clock_lamport) {
            l_clock = message_in.clock_lamport;
        }
        l_clock = l_clock + 1;

        /*print message*/
        printf("%s, %d, RECV (%s),", name_p, message_in.clock_lamport, message_in.origin); 
        print_operation(message_in.action);
    }
}
    
/* Establecer ip y puerto de los clientes*/
void set_ip_port (char* ip, unsigned int port)
{
    int sockfd;
    struct sockaddr_in clientaddr;
    struct message message_in;
    pthread_t thread;
    
    /* clear structure */
    memset(&clientaddr, 0, sizeof(clientaddr));
    
    /* assign IP, SERV_PORT, IPV4 */
    clientaddr.sin_family      = AF_INET; 
    clientaddr.sin_addr.s_addr = inet_addr(ip); 
    clientaddr.sin_port        = htons(port);

    /* socket creation */
    sockfd = socket(AF_INET, SOCK_STREAM, 0); 
    if (sockfd == -1) { 
        err(EXIT_FAILURE, "error: socket creation failed\n");
    }

    /* try to connect the client socket to server socket */
    if (connect(sockfd, (struct sockaddr*)&clientaddr, sizeof(clientaddr)) != 0) { 
        err(EXIT_FAILURE, "error: connection with server failed\n");
    }
    servfd = sockfd;

    /*threads creation*/
    if (pthread_create(&thread, NULL, handle_connection, &sockfd) != 0) {
        err(EXIT_FAILURE, "error: cannot create thread\n");
    }
}

/* Establecer ip y puerto del servidor*/
void set_on_proxy_server(char* ip, unsigned int port)
{
    int sockfd, i;
    unsigned int len;
    struct sockaddr_in servaddr;
    struct sockaddr_in client;
    const int enable = 1;

    /* clear structure */
    memset(&servaddr, 0, sizeof(servaddr));
    
    /* assign IP, SERV_PORT, IPV4 */
    servaddr.sin_family      = AF_INET; 
    servaddr.sin_addr.s_addr = inet_addr(ip); 
    servaddr.sin_port        = htons(port);
    
    /* socket creation */
    sockfd = socket(AF_INET, SOCK_STREAM, 0); 
    if (sockfd == -1) { 
        err(EXIT_FAILURE, "error: socket creation failed\n");
    }

    /* socket reutilization */
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
       err(EXIT_FAILURE, "error: setsockopt(SO_REUSEADDR) failed\n");
    }
    
    /* Bind socket */
    if ((bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr))) != 0) { 
        err(EXIT_FAILURE, "error: socket bind failed\n");
    } 
  
    /* Listen */
    if ((listen(sockfd, BACKLOG)) != 0) { 
        err(EXIT_FAILURE, "error: socket listen failed\n");
    }
    
    for (i = 0; i < PROCESSES; i++){
        /* Accept the data from incoming socket*/
        len = sizeof(client);
        connfd[i] = accept(sockfd, (struct sockaddr *)&client, &len); 

        if (connfd[i] < 0) { 
            err(EXIT_FAILURE, "error: connection not accepted\n");
        }

    }
}

// Obtiene el valor del reloj de lamport.
// Utilízalo cada vez que necesites consultar el tiempo.
// Esta función NO puede realizar ningún tiempo de comunicación (sockets)
int get_clock_lamport()
{
    return l_clock;
}

// Notifica que está listo para realizar el apagado (READY_TO_SHUTDOWN)
void notify_ready_shutdown()
{
    connect_socket = servfd;
    send_message(0);
}

// Notifica una orden de apagado
void notify_shutdown_now(int process)
{
    connect_socket = connfd[process];
    send_message(1);
}

// Notifica que va a realizar el shutdown correctamente (SHUTDOWN_ACK)
void notify_shutdown_ack()
{
    connect_socket = servfd;
    send_message(2);
}