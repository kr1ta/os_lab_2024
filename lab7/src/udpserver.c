#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <getopt.h>
#include <stdbool.h>

#define SADDR struct sockaddr
#define SLEN sizeof(struct sockaddr_in)

int main(int argc, char *argv[])
{

  int port = -1;
  int bufsize = -1;
  while (true)
  {
    int current_optind = optind ? optind : 1;
    static struct option options[] = {
      {"bufsize", required_argument, 0, 0},
      {"port", required_argument, 0, 0},
      {0, 0, 0, 0}
    };


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
        if (bufsize < 0)
        {
          printf("bufsize should be a positive integer!\n");
          return -1;
        }
        break;
      case 1:
        port = atoi(optarg);
        if (port < 1 || port > 65535)
        {
          printf("port should be an integer between 1 and 65535!\n");
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

  if (bufsize == -1 || port == -1)
  {
    fprintf(stderr, "Using: %s --bufsize 100 --port 5050\n",
            argv[0]);
    return 1;
  }

  int sockfd, n;
  char mesg[bufsize], ipadr[16];
  struct sockaddr_in servaddr;
  struct sockaddr_in cliaddr;

  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
  {
    perror("socket problem");
    exit(1);
  }

  memset(&servaddr, 0, SLEN);
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(port);

  if (bind(sockfd, (SADDR *)&servaddr, SLEN) < 0)
  {
    perror("bind");
    exit(1);
  }
  printf("SERVER starts...\n");

  while (1)
  {
    unsigned int len = SLEN;

    if ((n = recvfrom(sockfd, mesg, bufsize, 0, (SADDR *)&cliaddr, &len)) < 0)
    {
      perror("recvfrom");
      exit(1);
    }
    mesg[n] = 0;

    printf("REQUEST %s      FROM %s : %d\n", mesg,
           inet_ntop(AF_INET, (void *)&cliaddr.sin_addr.s_addr, ipadr, 16),
           ntohs(cliaddr.sin_port));

    if (sendto(sockfd, mesg, n, 0, (SADDR *)&cliaddr, len) < 0)
    {
      perror("sendto");
      exit(1);
    }
  }
}