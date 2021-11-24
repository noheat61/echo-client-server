#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <set>
#include <iso646.h>

char message[1000];

int eflag = 0, bflag = 0;
std::set<int> socket_fd;

void ERROR(const char *str)
{
    perror(str);
    exit(1);
}

void *myFunc(int socket)
{
    while (1)
    {
        //recv
        int len = read(socket, message, sizeof(message) - 1);
        if (len <= 0)
        {
            socket_fd.erase(socket);
            break;
        }
        message[len] = '\x00';
        printf("%s", message);

        //send
        if (eflag and (not bflag))
        {
            if (write(socket, message, strlen(message)) == -1)
                ERROR("write() fail");
        }
        //broadcast
        if (bflag)
        {
            for (int fd : socket_fd)
            {
                if (write(fd, message, strlen(message)) == -1)
                    ERROR("write() fail");
            }
        }
    }
}

int main(int argc, char *argv[])
{
    //argc must be 2 ~ 4
    if ((argc < 2) or (argc > 4))
    {
        printf("syntax : echo-server <port> [-e[-b]]\n");
        printf("sample : echo-server 1234 -e -b\n");
        return -1;
    }
    //argv[2] must be "-e"
    if (argc >= 3)
    {
        if (strncmp(argv[2], "-e", sizeof("-e")))
        {
            printf("syntax : echo-server <port> [-e[-b]]\n");
            printf("sample : echo-server 1234 -e -b\n");
            return -1;
        }
        eflag = 1;
    }
    //argv[3] must be "-b"
    if (argc >= 4)
    {
        if (strncmp(argv[3], "-b", sizeof("-b")))
        {
            printf("syntax : echo-server <port> [-e[-b]]\n");
            printf("sample : echo-server 1234 -e -b\n");
            return -1;
        }
        bflag = 1;
    }

    //create socket
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1)
        ERROR("socket() fail");

    //configure socket
    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(struct sockaddr_in));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = htons(atoi(argv[1]));

    //bind
    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1)
        ERROR("bind() fail");

    //listen
    if (listen(server_socket, 100) == -1)
        ERROR("listen() fail");

    while (1)
    {
        //accept
        struct sockaddr_in client_address;
        unsigned int client_address_length = sizeof(client_address);
        int connection_socket = accept(server_socket, (struct sockaddr *)&client_address, &client_address_length);
        if (connection_socket < 0)
            ERROR("accept() fail");

        //myFunc에서 실행
        std::thread thread(myFunc, connection_socket);
        socket_fd.insert(connection_socket);

        //join 대신 detach를 사용하면 이 스레드를 별개의 스레드로 분리하여 종료할 때까지 기다리지 않음
        thread.detach();
    }
    close(server_socket);
}
