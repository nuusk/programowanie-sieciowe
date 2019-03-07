#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>

#define MAX 512

int main()
{
    char buff[MAX];                                       //for tcp connections
    int network_socket = socket(AF_INET, SOCK_STREAM, 0); //create a socket

    struct sockaddr_in server_address;   //specify an address for the socket to connect to
    server_address.sin_family = AF_INET; //now we know what type of address we work with
    server_address.sin_port = htons(9002);

    server_address.sin_addr.s_addr = INADDR_ANY; //now we modify the actual address

    //connect
    int connection_status = connect(network_socket, (struct sockaddr *)&server_address, sizeof(server_address));
    if (connection_status == -1)
    {
        perror("Error conecting to the remote socket");
        exit(1);
    }

    //receive data from the server_address

    read(network_socket, buff, MAX);
    printf("Otrzymano: %s", buff);

    close(network_socket);

    return 0;
}
