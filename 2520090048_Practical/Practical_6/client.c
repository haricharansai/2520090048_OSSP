#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#define BUFFER_SIZE 100

int main()
{
     int write_fd, read_fd;
     char message[BUFFER_SIZE];
     char response[BUFFER_SIZE];

     write_fd = open("client_to_server", O_WRONLY);

     read_fd = open("server_to_client", O_RDONLY);

     printf("Client started.\n");

     while (1)
     {
          printf("Enter message: ");
          fgets(message, BUFFER_SIZE, stdin);

          message[strcspn(message, "\n")] = '\0';

          write(write_fd, message, strlen(message) + 1);

          int n = read(read_fd, response, BUFFER_SIZE - 1);

          if (n > 0)
          {
               response[n] = '\0';
               printf("Server: %s\n", response);
          }

          if (strcmp(message, "exit") == 0)
               break;
     }

     close(write_fd);
     close(read_fd);

     return 0;
}