#ifndef LOG_MONITOR_H_
#define LOG_MONITOR_H_

struct TradingInfo {
    int trader;
    int decision;
    double usd;
    double btc;
};

void put_info(int trader, int decision, double usd, double btc);
struct TradingInfo get_info();

#endif // LOG_MONITOR_H_
