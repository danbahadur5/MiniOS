#ifndef SYNC_H
#define SYNC_H
void init_producer_consumer();
void producer();
void consumer();
void reader(int id);
void writer(int id);
void bankers_algorithm();
#endif