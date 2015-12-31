
#ifndef _DHT22_H
#define _DHT22_H

#include <stdbool.h>

#define DHT_ERROR_TIMEOUT -1
#define DHT_ERROR_CHECKSUM -2
#define DHT_ERROR_ARGUMENT -3
#define DHT_ERROR_GPIO -4
#define DHT_SUCCESS 0


int pi_2_dht_read(float* humidity, float* temperature);

#endif

