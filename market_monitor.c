#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>

#include "market_monitor.h"

static pthread_mutex_t exclusao_mutua = PTHREAD_MUTEX_INITIALIZER;
double last_price = 0;
// TODO: Maybe an alarm if goes too high or too low?

void put_price(double price) {
  // TODO: Broadcast
  pthread_mutex_lock( &exclusao_mutua);
  last_price = price;
  pthread_mutex_unlock( &exclusao_mutua);
}

double get_price() {
  pthread_mutex_lock( &exclusao_mutua);
  double price = last_price;
  pthread_mutex_unlock( &exclusao_mutua);
  return price;
}
