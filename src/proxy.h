enum operations {
    READY_TO_SHUTDOWN = 0,
    SHUTDOWN_NOW,
    SHUTDOWN_ACK
};

struct message {
    char origin[20];
    enum operations action;
    unsigned int clock_lamport;
};

void set_name (char name[2]);
void print_operation(int n);
void send_message();
void listen_message(int n);
void * handle_connection(void *p_client_socket);
void set_ip_port (char* ip, unsigned int port);
void set_on_proxy_server(char* ip, unsigned int port);
int get_clock_lamport();
void notify_ready_shutdown();
void notify_shutdown_ack();
void notify_shutdown_now(int process);
void save_sockets();