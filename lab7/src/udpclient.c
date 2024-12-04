#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>

#include <arpa/inet.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <getopt.h>
#include <stdbool.h>

#define SADDR struct sockaddr
#define SLEN sizeof(struct sockaddr_in)

int main(int argc, char **argv)
{

  int bufsize = -1;
  int port = -1;
  char ip[255] = {'\0'};

  while (true)
  {
    int current_optind = optind ? optind : 1;
    static struct option options[] = {
      {"bufsize", required_argument, 0, 0},
      {"ip", required_argument, 0, 0},
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
    fprintf(stderr, "Using: %s --bufsize 100 --ip 0.0.0.0 --port 5050\n",
            argv[0]);
    return 1;
  }

  int sockfd, n;
  char sendline[bufsize], recvline[bufsize + 1];
  struct sockaddr_in servaddr;
  struct sockaddr_in cliaddr;

  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(port);

  if (inet_pton(AF_INET, ip, &servaddr.sin_addr) < 0)
  {
    perror("inet_pton problem");
    exit(1);
  }
  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
  {
    perror("socket problem");
    exit(1);
  }

  write(1, "Enter string\n", 13);

  while ((n = read(0, sendline, bufsize)) > 0)
  {
    if (sendto(sockfd, sendline, n, 0, (SADDR *)&servaddr, SLEN) == -1)
    {
      perror("sendto problem");
      exit(1);
    }

    if (recvfrom(sockfd, recvline, bufsize, 0, NULL, NULL) == -1)
    {
      perror("recvfrom problem");
      exit(1);
    }

    printf("REPLY FROM SERVER= %s\n", recvline);
  }
  close(sockfd);
}