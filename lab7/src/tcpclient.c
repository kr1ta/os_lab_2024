#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <getopt.h>
#include <stdbool.h>

#define SADDR struct sockaddr
#define SIZE sizeof(struct sockaddr_in)

int main(int argc, char *argv[])
{
  int bufsize = -1;
  int port = -1;
  char ip[255] = {'\0'};

  while (true)
  {
    int current_optind = optind ? optind : 1;

    static struct option options[] = {{"bufsize", required_argument, 0, 0},
                                      {"ip", required_argument, 0, 0},
                                      {"port", required_argument, 0, 0},
                                      {0, 0, 0, 0}};

    int option_index = 0;
    int c = getopt_long(argc, argv, "", options, &option_index);

    if (c == -1)
      break;

    switch (c)
    {
    case 0:
    {
      switch (option_index)
      {
      case 0:
        bufsize = atoi(optarg);
        if (bufsize < 1 || bufsize > 65535)
        {
          printf("bufsize should be an integer!\n");
          return -1;
        }
        break;
      case 1:
        memcpy(ip, optarg, strlen(optarg));
        break;
      case 2:
        port = atoi(optarg);
        if (port < 1 || port > 65535)
        {
          printf("port should be an integer!\n");
          return -1;
        }
        break;
      default:
        printf("Index %d is out of options\n", option_index);
      }
    }
    break;

    case '?':
      printf("Unknown argument\n");
      break;
    default:
      fprintf(stderr, "getopt returned character code 0%o?\n", c);
    }
  }

  if (bufsize == -1 || port == -1 || strlen(ip) == 0)
  {
    fprintf(stderr, "Using: %s --bufsize 100 --ip 127.0.0.0 --port 5050\n",
            argv[0]);
    return 1;
  }

  int fd;
  int nread;
  char buf[bufsize];
  struct sockaddr_in servaddr;

  if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    perror("socket creating");
    exit(1);
  }

  memset(&servaddr, 0, SIZE);
  servaddr.sin_family = AF_INET;

  if (inet_pton(AF_INET, ip, &servaddr.sin_addr) <= 0)
  {
    perror("bad address");
    exit(1);
  }

  servaddr.sin_port = htons(port);

  if (connect(fd, (SADDR *)&servaddr, SIZE) < 0)
  {
    perror("connect");
    exit(1);
  }

  write(1, "Input message to send\n", 22);
  while ((nread = read(0, buf, bufsize)) > 0)
  {
    if (write(fd, buf, nread) < 0)
    {
      perror("write");
      exit(1);
    }
  }

  close(fd);
  exit(0);
}