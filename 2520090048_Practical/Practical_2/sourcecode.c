#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

#define BUFFER_SIZE 1024

int main()
{
     int source_fd, dest_fd;
     char buffer[BUFFER_SIZE];
     int bytes_read, bytes_written;

     source_fd = open("source.txt", O_RDONLY);

     if (source_fd < 0)
     {
          perror("Error opening source file");
          exit(1);
     }

     dest_fd = open("destination.txt",
                    O_WRONLY | O_CREAT | O_TRUNC,
                    0644);

     if (dest_fd < 0)
     {
          perror("Error creating destination file");
          close(source_fd);
          exit(1);
     }

     while ((bytes_read = read(source_fd, buffer, BUFFER_SIZE)) > 0)
     {
          bytes_written = write(dest_fd, buffer, bytes_read);

          if (bytes_written != bytes_read)
          {
               perror("Write error");
               close(source_fd);
               close(dest_fd);
               exit(1);
          }
     }

     close(source_fd);
     close(dest_fd);

     printf("File copied successfully.\n");

     return 0;
}