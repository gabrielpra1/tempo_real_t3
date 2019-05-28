#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>

#include "market_monitor.h"

// TODO: Create monitor for read-writer
// TODO: Create monitor for market-data

// Traders wait broadcast to see new data has arrived and then act on it
// Traders decisions are stored in a single array and read by one thread: readers-writers problem
void trader_1() {
  while(1) {
  }
}

void trader_2() {
  while(1) {
  }
}

void printer() {
  while(1) {
  }
}

void market() {
  // read CSV
  // release data every second, broadcasting signal for traders
}

// gcc -o controller controller.c udp.c -lrt -lpthread
// ./controller localhost 4000
int main(int argc, char *argv[])
{
  pthread_t t1, t2, t3;
  pthread_create(&t1, NULL, (void *) market, NULL);
  pthread_create(&t2, NULL, (void *) trader_1, NULL);
  pthread_create(&t3, NULL, (void *) trader_2, NULL);

  printer();
  return 0;
}
