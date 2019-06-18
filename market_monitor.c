#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>

#include "market_monitor.h"

static pthread_mutex_t mutual_exclusion = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t has_price = PTHREAD_COND_INITIALIZER;
static double last_price = -1;

void put_price(double price) {
  pthread_mutex_lock(&mutual_exclusion);

  last_price = price;
  pthread_cond_broadcast(&has_price);

  pthread_mutex_unlock(&mutual_exclusion);
}

double get_price() {
  pthread_mutex_lock(&mutual_exclusion);

  pthread_cond_wait(&has_price, &mutual_exclusion);
  double price = last_price;

  pthread_mutex_unlock(&mutual_exclusion);
  return price;
}
