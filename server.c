#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

int main()
{
  unsigned char buffer[100];

  struct sockaddr_in  serverInfo = {0};
  serverInfo.sin_family = AF_INET;
  serverInfo.sin_addr.s_addr = 0;
  serverInfo.sin_port = htons(5555);


  struct sockaddr_in  clientInfo = {0};
  int clientSize = 0;
  int fd = socket(AF_INET, SOCK_STREAM, 0);
  if (fd == -1)
  {
    perror("socket");
    return -1;
  }
  int opt = 1;
  if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
  {
    perror("setsockopt");
    close(fd);
    return -1;
  }
  if (bind(fd, (struct sockaddr *)&serverInfo, sizeof(serverInfo)) == -1)
  {
    perror("bind");
    close(fd);
    return -1;
  }
  if (listen(fd, 0) == -1)
  {
    perror("listen");
    close(fd);
    return -1;
  }
  while (1)
  {
    int cfd = accept(fd, (struct sockaddr *)&clientInfo, &clientSize);
    if (cfd == -1)
    {
      perror("accept");
      close(fd);
      return -1;
    }
    bzero(buffer, sizeof(buffer));
    read(cfd, buffer, sizeof(buffer));
    write(1, buffer, sizeof(buffer));
    close(cfd);
  }
  close(fd);
}
