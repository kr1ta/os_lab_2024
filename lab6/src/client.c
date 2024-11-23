#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <errno.h>
#include <getopt.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>

#include "multmodulo.h"

struct Server {
  char ip[255];
  int port;
};

bool ConvertStringToUI64(const char *str, uint64_t *val) {
  char *end = NULL;
  unsigned long long i = strtoull(str, &end, 10);
  if (errno == ERANGE) {
    fprintf(stderr, "Out of uint64_t range: %s\n", str);
    return false;
  }

  if (errno != 0)
    return false;

  *val = i;
  return true;
}

void parseServers(struct Server* to, unsigned int* servers_num, char* servers) {
  FILE *file = fopen(servers, "r");
  if (file == NULL) {
      perror("Unable to open file");
      return;
  }

  char line[256];

  while (fgets(line, sizeof(line), file)) {
      unsigned int port;
      sscanf(line, "%*[^:]:%d", &to[(*servers_num)++].port);
  }

  fclose(file);

  for (int i = 0; i < *servers_num; i++) {
    memcpy(to[i].ip, "127.0.0.1", sizeof("127.0.0.1"));
  }
}

int main(int argc, char **argv) {
  uint64_t k = -1;
  uint64_t mod = -1;
  char servers[255] = {'\0'};

  while (true) {
    int current_optind = optind ? optind : 1;

    static struct option options[] = {{"k", required_argument, 0, 0},
                                      {"mod", required_argument, 0, 0},
                                      {"servers", required_argument, 0, 0},
                                      {0, 0, 0, 0}};

    int option_index = 0;
    int c = getopt_long(argc, argv, "", options, &option_index);

    if (c == -1)
      break;
      
    switch (c) {
    case 0: {
      switch (option_index) {
      case 0:
        ConvertStringToUI64(optarg, &k);
        break;
      case 1:
        ConvertStringToUI64(optarg, &mod);
        break;
      case 2:
        memcpy(servers, optarg, strlen(optarg));
        break;
      default:
        printf("Index %d is out of options\n", option_index);
      }
    } break;

    case '?':
      printf("Arguments error\n");
      break;
    default:
      fprintf(stderr, "getopt returned character code 0%o?\n", c);
    }
  }

  if (k == -1 || mod == -1 || !strlen(servers)) {
    fprintf(stderr, "Using: %s --k 1000 --mod 5 --servers /path/to/file\n",
            argv[0]);
    return 1;
  }

  unsigned int servers_num = 0;
  struct Server *to = (struct Server*)malloc(sizeof(struct Server) * 10);
  parseServers(to, &servers_num, servers);

  uint64_t* part_answer = (uint64_t *)malloc(sizeof(uint64_t) * servers_num);
  uint64_t answer = 1;
  for (int i = 0; i < servers_num; i++) {
    struct hostent *hostname = gethostbyname(to[i].ip);
    if (hostname == NULL) {
      fprintf(stderr, "gethostbyname failed with %s\n", to[i].ip);
      exit(1);
    } else {
      fprintf(stdout, "[%d] addr:port = %s:%d\n", i, to[i].ip, to[i].port);
    }

    // настройка сокета и сервера к которому подключимся
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(to[i].port);
    server.sin_addr.s_addr = inet_addr(to[i].ip);
  
    int sck = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); // тисипи
    if (sck < 0) {
      fprintf(stderr, "Socket creation failed!\n");
      exit(1);
    }
    if (connect(sck, (struct sockaddr *)&server, sizeof(server)) < 0) {
      fprintf(stderr, "Connection failed\n");
      exit(1);
    }

    uint64_t begin = i*(k/servers_num) + 1;
    uint64_t end = (i == servers_num - 1) ? k : (i+1)*(k/servers_num);

    char task[sizeof(uint64_t) * 3];
    memcpy(task, &begin, sizeof(uint64_t));
    memcpy(task + sizeof(uint64_t), &end, sizeof(uint64_t));
    memcpy(task + 2 * sizeof(uint64_t), &mod, sizeof(uint64_t));
    if (send(sck, task, sizeof(task), 0) < 0) {
      fprintf(stderr, "Send failed\n");
      exit(1);
    }

    char response[sizeof(uint64_t)];
    if (recv(sck, response, sizeof(response), 0) < 0) {
      fprintf(stderr, "Recieve failed\n");
      exit(1);
    }
    
    memcpy(&part_answer[i], response, sizeof(uint64_t));
    answer *= part_answer[i];

    close(sck);
  }

  free(to);
  printf("answer = %lu\n", answer);

  return 0;
}