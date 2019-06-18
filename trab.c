#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>

#include "csvparser.h"
#include "market_monitor.h"
#include "log_monitor.h"

#define NUMBER_POINTS 20000
#define INITIAL_AMOUNT 10000 // USD
#define TRADER_2_POINTS 5
#define TRADER_3_POINTS 5

// Decisions:
//  -1: SELL
//   0: HOLD
//   1: BUY

double trader_1_usd = INITIAL_AMOUNT;
double trader_1_btc = 0;
int decision_1[NUMBER_POINTS];
double last_price = -1;
int decision_1_index = 0;

double trader_2_usd = INITIAL_AMOUNT;
double trader_2_btc = 0;
int decision_2[NUMBER_POINTS];
double last_prices[5];
int decision_2_index = 0;
int last_prices_index = 0;
int is_full = 0;

double trader_3_usd = INITIAL_AMOUNT;
double trader_3_btc = 0;
int decision_3[NUMBER_POINTS];
double last_prices_3[5];
int decision_3_index = 0;
int last_prices_3_index = 0;
int is_full_3 = 0;
double bought_price = 0;

// Trader 1: If price goes up, sell eveything. If it goes down, buy everything.
void trader_1() {
  while(1) {
    double price = get_price();
    if (price > last_price && trader_1_btc > 0) {
      // sell
      decision_1[decision_1_index] = -1;
      double btc = trader_1_btc;
      trader_1_usd += btc * price;
      trader_1_btc = 0;
    } else if (price < last_price && trader_1_usd > 0) {
      // BUY
      decision_1[decision_1_index] = 1;
      double usd = trader_1_usd;
      trader_1_btc += (usd / price);
      trader_1_usd = 0;
    } else {
      // Hold
      decision_1[decision_1_index] = 0;
    }

    last_price = price;
    put_info(1, decision_1[decision_1_index], trader_1_usd, trader_1_btc);
    decision_1_index++;
  }
}

int is_greater(double price, double *last_prices, int count) {
  int greater = 1;
  for (int i = 0; i < count; i++) {
    if (price <= last_prices[i])
      greater = 0;
  }
  return greater;
}

int is_smaller(double price, double *last_prices, int count) {
  int smaller = 1;
  for (int i = 0; i < count; i++) {
    if (price >= last_prices[i])
      smaller = 0;
  }
  return smaller;
}

double average(double *last_prices, int count) {
  double sum = 0;
  for (int i = 0; i < count; i++) {
    sum += last_prices[i];
  }
  return sum / count;
}

// Trader 2: If price is higher than last 5 prices, sell everything.
// If it's less than last 5 prices, buy everything.
void trader_2() {
  while(1) {
    double price = get_price();

    // Only act after has recorded TRADER_2_POINTS points
    if (is_full == 1) {
      int greater = is_greater(price, last_prices, TRADER_2_POINTS);
      int smaller = is_smaller(price, last_prices, TRADER_2_POINTS);

      if (greater && trader_2_btc > 0) {
        // Sell
        decision_2[decision_2_index] = -1;
        double btc = trader_2_btc;
        trader_2_usd += btc * price;
        trader_2_btc = 0;
      } else if (smaller && trader_2_usd > 0) {
        // Buy
        decision_2[decision_2_index] = 1;
        double usd = trader_2_usd;
        trader_2_btc += (usd / price);
        trader_2_usd = 0;
      } else {
        // Hold
        decision_2[decision_2_index] = 0;
      }

      last_prices_index = last_prices_index % TRADER_2_POINTS;
      last_prices[last_prices_index++] = price;
    } else {
      decision_2[decision_2_index] = 0;
      last_prices[last_prices_index++] = price;
      if (last_prices_index == TRADER_2_POINTS)
        is_full = 1;
    }

    put_info(2, decision_2[decision_2_index], trader_2_usd, trader_2_btc);
    decision_2_index++;
  }
}

// Trader 3 -> only sell if it's higher than bought price, and only buy if it's lower than recent average
void trader_3() {
  while(1) {
    double price = get_price();

    // Only act after has recorded TRADER_3_POINTS points
    if (is_full_3 == 1) {
      double avg = average(last_prices_3, TRADER_3_POINTS);

      if (price > bought_price && trader_3_btc > 0) {
        // Sell
        decision_3[decision_3_index] = -1;
        double btc = trader_3_btc;
        trader_3_usd += btc * price;
        trader_3_btc = 0;
      } else if (price < avg && trader_3_usd > 0) {
        // Buy
        decision_3[decision_3_index] = 1;
        double usd = trader_3_usd;
        trader_3_btc += (usd / price);
        trader_3_usd = 0;
        bought_price = price;
      } else {
        // Hold
        decision_3[decision_3_index] = 0;
      }

      last_prices_3_index = last_prices_3_index % TRADER_3_POINTS;
      last_prices_3[last_prices_3_index++] = price;
    } else {
      decision_3[decision_3_index] = 0;
      last_prices_3[last_prices_3_index++] = price;
      if (last_prices_3_index == TRADER_3_POINTS)
        is_full_3 = 1;
    }

    put_info(3, decision_3[decision_3_index], trader_3_usd, trader_3_btc);
    decision_3_index++;
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
  for (int j = NUMBER_POINTS - 1; j >= 0; j--) {
    printf("Open price: %f\n", prices[j]);
    put_price(prices[j]);
    sleep(1);
  }
}

// gcc -o trab trab.c csvparser.c market_monitor.c log_monitor.c -lpthread
// ./trab
int main(int argc, char *argv[])
{
  pthread_t t1, t2, t3, t4;
  pthread_create(&t1, NULL, (void *) market, NULL);
  pthread_create(&t2, NULL, (void *) trader_1, NULL);
  pthread_create(&t3, NULL, (void *) trader_2, NULL);
  pthread_create(&t4, NULL, (void *) trader_3, NULL);

  printer();
  return 0;
}
