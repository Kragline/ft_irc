#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

/* --- NETWORK SHIT ---*/
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <poll.h>
/* --------------------*/

#define MAX_USERS 256
#define WELCOME_001 "001 <nick> :Welcome to the IRC Network <nick>\r\n\0"
#define WELCOME_002 "002 <nick> :Your host is server.name, running version 1.0\r\n\0"
#define WELCOME_003 "003 <nick> :This server was created today\r\n\0"
#define WELCOME_004 "004 <nick> server.name 1.0 o o\r\n\0"
#define JOIN_461 ":localhost 461 <nick> JOIN :Not enough parameters\r\n\0"

typedef enum Role
{
  OPERATOR,
  REGULAR
} role_e;

typedef struct clientstate_s // Change this shit to class list 
{
  int           fd;
  role_e        role;

  char *nick;
  char *user;
  char *real_name;
} clientstate_t;


clientstate_t clientState[MAX_USERS];

void  init_clients(void)
{
  for (int i = 0; i < MAX_USERS; i++)
  {
    clientState[i].fd = -1;
    clientState[i].role = REGULAR;
    clientState[i].nick = NULL;
    clientState[i].user = NULL;
    clientState[i].real_name = NULL;
  }
}

int  init_server(void)
{ 
  int opt = 1;
  struct sockaddr_in  serverInfo = {0};
  serverInfo.sin_family = AF_INET;
  serverInfo.sin_addr.s_addr = 0;
  serverInfo.sin_port = htons(5555);

  int fd = socket(AF_INET, SOCK_STREAM, 0);
  if (fd == -1)
  {
    perror("socket");
    return -1;
  }
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
  return fd;
}

ssize_t recv_line(int fd, char *buf)
{
  size_t i = 0;
  char c;
  ssize_t n;

  while (i < 512)
  {
    n = read(fd, &c, 1);
    if (n <= 0)
      return n; 
    buf[i] = c;
    if (c == '\n' && buf[i - 1] == '\r')
    {
      buf[i + 1] = '\0';
      return i;
    }
    i++;
  }
  buf[i] = '\0';
  return i;
}

void  handle_nick(char *buf, int index)
{
  char temp[9];
  int i;

  for (i = 0; buf[i] != '\r'; i++)
    temp[i] = buf[i];
  temp[i] = '\0';
  clientState[index].nick = strdup(temp);
  printf("nick: %s\n", clientState[index].nick);
}

void  handle_user(char *buf, int index)
{
  char temp[9];
  int i;

  for (i = 0; buf[i] != ' '; i++)
    temp[i] = buf[i];
  temp[i] = '\0';
  clientState[index].user = strdup(temp);
  
 for (i = 0; buf[i] != ' '; i++)
    temp[i] = buf[i];
  temp[i] = '\0';
  clientState[index].real_name = strdup(temp);

  printf("user: %s\n", clientState[index].user);
  printf("real_name: %s\n", clientState[index].real_name);
}

void  handle_cap(int cfd, char *buffer, int i)
{
  recv_line(cfd, buffer);
  if (strncmp(buffer, "CAP LS", 6) == 0)
    write(cfd, "CAP * LS :\r\n", 13);

  while (strcmp("CAP END\r\n", buffer) != 0)
    recv_line(cfd, buffer);

  recv_line(cfd, buffer);
  if (strncmp("NICK", buffer, 4) == 0)
    handle_nick(buffer + 5, i);

  recv_line(cfd, buffer);
  if (strncmp("USER", buffer, 4) == 0)
    handle_user(buffer + 5, i);

  free(clientState[i].nick);      // Remove Later
  free(clientState[i].user);      // Remove Later
  free(clientState[i].real_name); // Remove Later
  write(cfd, WELCOME_001, strlen(WELCOME_001));
  write(cfd, WELCOME_002, strlen(WELCOME_002));
  write(cfd, WELCOME_003, strlen(WELCOME_003));
  write(cfd, WELCOME_004, strlen(WELCOME_004));
}

int main()
{
  int fd;                               // Main connection socket file descriptor
  int cfd;                              // Temp client socket file descriptor
  int i = 0;                            // Index for client structure
  char buffer[512];                     // Buffer for client messages
  bzero(buffer, sizeof(buffer)); 
                              
  struct sockaddr_in  clientInfo = {0}; // Don't touch it
  socklen_t clientSize = 0;
  
  fd = init_server();
  if (fd == -1)
    return -1;

  init_clients();
 
  while (1) // Main loop
  {
    cfd = accept(fd, (struct sockaddr *)&clientInfo, &clientSize);
    if (cfd == -1)
    {
      perror("accept");
      close(fd);
      return -1;
    }

    handle_cap(cfd, buffer, i);
    clientState[i].fd = cfd;
    read(cfd, buffer, sizeof(buffer));
  }
  close(fd);
  return 0;
}
