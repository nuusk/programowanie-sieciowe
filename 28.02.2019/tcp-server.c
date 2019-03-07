#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>

#define MAX 512

// RUNNING: ./tcp-server <next_address> <next_port>

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        printf("Insufficient number of arguments.\nUse %s <next_address> <next_port>\n", argv[0]);
        exit(1);
    }

    char *next_ip = argv[1];
    char *next_port = argv[2];

    printf("Next computer ip is %s\n", next_ip);
    printf("Next computer port is %s\n", next_port);

    //what the server has to say
    char server_message[256] = "Hey. This is server speaking."; //for udp connections
    char buff[MAX] = "elo elo 3 2 0";                           //for tcp connections
    char client_message[MAX];

    //create the server socket
    int server_socket;
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1)
    {
        perror("Error creating socket");
        exit(1);
    }

    //define server address
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(9002);
    server_address.sin_addr.s_addr = INADDR_ANY;

    //define server address
    struct sockaddr_in next_address;
    next_address.sin_family = AF_INET;
    next_address.sin_port = htons(9002);
    next_address.sin_addr.s_addr = INADDR_ANY;

    //bind the socket to our specified IP and port
    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)))
    {
        perror("Error binding socket to specified IP and port");
        exit(1);
    }

    listen(server_socket, 5);

    //accept the connection
    int client_socket;
    while ((client_socket = accept(server_socket, NULL, NULL)) != -1)
    {
        if (client_socket == -1)
        {
            perror("Accept connection failed");
            exit(1);
        }

        if (write(client_socket, buff, MAX) == -1)
        {
            perror("Error writing to the client socket");
            exit(1);
        }

        bzero(client_message, sizeof(client_message));
        if (read(client_socket, client_message, MAX) == -1)
        {
            perror("Error while reading socket");
            exit(1);
        }
        printf("Otrzymano: %s\n", client_message);
    }

    printf("end of tcp.");

    ////// END OF TCP ///////////

    // close(server_socket);
    return 0;
}
