#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>

#include "csvparser.h"
#include "market_monitor.h"
#include "log_monitor.h"

#define NUMBER_POINTS 1000
#define INITIAL_AMOUNT 10000 // USD

// Decisions:
//  -1: SELL
//   0: HOLD
//   1: BUY

double trader_1_usd = INITIAL_AMOUNT;
double trader_1_btc = 0;
int decision_1[NUMBER_POINTS];
double last_price = -1;
int decision_1_index = 0;

double trader_2_usd= INITIAL_AMOUNT;
double trader_2_btc = 0;
int decision_2[NUMBER_POINTS];
double last_prices[5];
int decision_2_index = 0;
int last_prices_index = 0;
int is_full = 0;

// TODO: Create monitor for read-writer

// Traders wait broadcast to see new data has arrived and then act on it
// Traders decisions are stored in a single array and read by one thread: readers-writers problem

// Trader 1: If price goes up, sell eveything. If it goes down, buy everything.
void trader_1() {
  while(1) {
    double price = get_price();
    if (price > last_price) {
      // sell
      decision_1[decision_1_index] = -1;
      double btc = trader_1_btc;
      trader_1_usd += btc * price;
      trader_1_btc = 0;
    } else if (price < last_price) {
      // BUY
      decision_1[decision_1_index] = 1;
      double usd = trader_1_usd;
      trader_1_btc += (usd / price);
      trader_1_usd = 0;
    }
    decision_1[decision_1_index] = 0;

    last_price = price;
    put_info(1, decision_1[decision_1_index], trader_1_usd, trader_1_btc);
    decision_1_index++;
  }
}

int is_greater(double price, double *last_prices) {
  int greater = 1;
  for (int i = 0; i < 5; i++) {
    if (price <= last_prices[i])
      greater = 0;
  }
  return greater;
}

int is_smaller(double price, double *last_prices) {
  int smaller = 1;
  for (int i = 0; i < 5; i++) {
    if (price >= last_prices[i])
      smaller = 0;
  }
  return smaller;
}

// Trader 2: If price is higher than last 5 prices, sell everything.
// If it's less than last 5 prices, buy everything.
void trader_2() {
  while(1) {
    double price = get_price();

    // Only act after has recorded 4 points
    if (is_full == 1) {
      int greater = is_greater(price, last_prices);
      int smaller = is_smaller(price, last_prices);

      if (greater) {
        // sell
        decision_2[decision_2_index] = -1;
        double btc = trader_2_btc;
        trader_2_usd += btc * price;
        trader_2_btc = 0;
      } else if (smaller) {
        // BUY
        decision_2[decision_2_index] = 1;
        double usd = trader_2_usd;
        trader_2_btc += (usd / price);
        trader_2_usd = 0;
      }

      last_prices_index = last_prices_index % 5;
      last_prices[last_prices_index++] = price;
    } else {
      last_prices[last_prices_index++] = price;
      if (last_prices_index == 5)
        is_full = 1;
    }

    decision_2[decision_2_index] = 0;
    put_info(2, decision_2[decision_2_index], trader_2_usd, trader_2_btc);
    decision_2_index++;
  }
}

void printer() {
  while(1) {
    struct TradingInfo info = get_info();
    printf("Trader %d. Decision: %d; USD: %lf; BTC: %lf\n", info.trader, info.decision, info.usd, info.btc);
  }
}

void market() {
  // read CSV
  int i =  0;
  float prices[NUMBER_POINTS];

  CsvParser *csvparser = CsvParser_new("data.csv", ",", 1);
  CsvRow *row;
  const CsvRow *header = CsvParser_getHeader(csvparser);

  while (i < NUMBER_POINTS && (row = CsvParser_getRow(csvparser)) ) {
      const char **rowFields = CsvParser_getFields(row);
      prices[i++] = atof(rowFields[3]);
      CsvParser_destroy_row(row);
  }
  CsvParser_destroy(csvparser);

  // release data every second, broadcasting signal for traders
  for (int j = 0; j < NUMBER_POINTS; j++) {
    printf("Open price: %f\n", prices[j]);
    put_price(prices[j]);
    sleep(0.2);
  }
}

// gcc -o trab trab.c csvparser.c market_monitor.c log_monitor.c -lpthread
// gcc -o controller controller.c udp.c -lrt -lpthread
// ./trab
int main(int argc, char *argv[])
{
  pthread_t t1, t2, t3;
  pthread_create(&t1, NULL, (void *) market, NULL);
  pthread_create(&t2, NULL, (void *) trader_1, NULL);
  pthread_create(&t3, NULL, (void *) trader_2, NULL);

  printer();
  return 0;
}
