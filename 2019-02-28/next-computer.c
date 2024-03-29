#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>

#define MAX 512

// RUNNING: ./next-computer <port>

int main(int argc, char **argv)
{
  if (argc < 2)
  {
    printf("Insufficient number of arguments.\nUse %s <port>\n", argv[0]);
    exit(1);
  }

  char *my_port = argv[1];

  printf("My port is %s\n", my_port);

  char greeting_message[MAX] = "I'm just the next one."; //what I have to say
  char client_message[MAX];                              // what client has to say

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
  server_address.sin_port = htons(my_port);
  server_address.sin_addr.s_addr = INADDR_ANY;

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

    if (write(client_socket, greeting_message, MAX) == -1)
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
    printf("I got: %s\n", client_message);
  }

  ////// END OF TCP ///////////
  printf("End of tcp...");

  // close(server_socket);
  return 0;
}
