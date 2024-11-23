#include "multmodulo.h"

uint64_t MultModulo(uint64_t a, uint64_t b, uint64_t mod) 
{
  uint64_t result = 0;
  a = a % mod;
  while (b > 0) 
  {
    if (b % 2 == 1)
      result = (result + a) % mod;
    a = (a * 2) % mod;
    b /= 2;
  }

  return result % mod;
}

// ./client --k 1000 --mod 5 --servers servers.txt
// ./server --port 20001 --tnum 4
// ./server --port 20002 --tnum 4