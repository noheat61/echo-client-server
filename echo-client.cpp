#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>

char receive_message[1000];
char message[1000];

void ERROR(const char *str)
{
    perror(str);
    exit(1);
}

void recv_from(int socket)
{
    while (1)
    {
        int len = read(socket, receive_message, sizeof(receive_message) - 1);
        if (len <= 0)
            ERROR("read() fail");
        receive_message[len] = '\x00';
        printf("%s", receive_message);
    }
}
void send_to(int socket)
{
    while (1)
    {
        fgets(message, sizeof(message), stdin);
        int len = write(socket, message, strlen(message));
        if (len <= 0)
            ERROR("write() fail");
    }
}

int main(int argc, char *argv[])
{
    //argc must be three
    if (argc != 3)
    {
        printf("syntax : echo-client <ip> <port>\n");
        printf("sample : echo-client 192.168.10.2 1234\n");
        return -1;
    }

    //create socket
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1)
        ERROR("socket() fail");

    //configure socket
    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(struct sockaddr_in));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(argv[1]);
    server_address.sin_port = htons(atoi(argv[2]));

    //connect
    if (connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1)
        ERROR("connect() fail");

    //send thread
    //recv thread
    std::thread t1(recv_from, client_socket);
    std::thread t2(send_to, client_socket);
    t1.join();
    t2.join();

    //close
    close(client_socket);
    return 0;
}
