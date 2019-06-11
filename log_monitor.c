#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>

#include "log_monitor.h"

static pthread_mutex_t mutual_exclusion = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t has_info = PTHREAD_COND_INITIALIZER;

static struct TradingInfo last_info;

void put_info(int trader, int decision, double usd, double btc) {
  pthread_mutex_lock(&mutual_exclusion);

  last_info = {
    .trader = trader,
    .decision = decision,
    .usd = usd,
    .btc = btc
  }
  pthread_cond_signal(&has_info);

  pthread_mutex_unlock(&mutual_exclusion);
}

struct TradingInfo get_info() {
  pthread_mutex_lock(&mutual_exclusion);

  while (last_info == NULL) {
    pthread_cond_wait(&has_info, &mutual_exclusion);
  }

  TradingInfo info = last_info;
  last_info = NULL;

  pthread_mutex_unlock(&mutual_exclusion);
  return info;
}
