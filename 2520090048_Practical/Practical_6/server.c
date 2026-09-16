#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#define BUFFER_SIZE 100

int main()
{
     int read_fd, write_fd;
     char buffer[BUFFER_SIZE];

     read_fd = open("client_to_server", O_RDONLY);

     write_fd = open("server_to_client", O_WRONLY);

     printf("Server started...\n");

     while (1)
     {
          memset(buffer, 0, BUFFER_SIZE);

          int n = read(read_fd, buffer, BUFFER_SIZE - 1);

          if (n > 0)
          {
               buffer[n] = '\0';

               printf("Client: %s\n", buffer);

               if (strcmp(buffer, "hello") == 0)
               {
                    strcpy(buffer, "Hello Client!");
               }
               else if (strcmp(buffer, "exit") == 0)
               {
                    strcpy(buffer, "Server shutting down...");
                    write(write_fd, buffer, strlen(buffer) + 1);
                    break;
               }
               else
               {
                    strcpy(buffer, "Server received your message.");
               }

               write(write_fd, buffer, strlen(buffer) + 1);
          }
     }

     close(read_fd);
     close(write_fd);

     return 0;
}