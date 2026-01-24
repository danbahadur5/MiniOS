#include "sync.h"
#include <stdio.h>
void init_producer_consumer() { 
    printf("Sync module initialized.\n"); 
}
void producer() { 
    printf("Producer executed.\n"); 
}
void consumer() { 
    printf("Consumer executed.\n"); 
}
void reader(int id) { 
    printf("Reader %d executed.\n", id); 
}
void writer(int id) { 
    printf("Writer %d executed.\n", id); 
}
void bankers_algorithm() { 
    printf("Banker’s Algorithm executed.\n"); 
}
