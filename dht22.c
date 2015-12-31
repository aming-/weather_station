// RaspberryPI - DHT22 sensor library - 2015.12.28 - @AM
// gcc -o dht22 -l rt dht22.c -l bcm2835 

#include <bcm2835.h>
#include <stdio.h>
//#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <sched.h>
#include <string.h>
//#include <sys/time.h>
//#include <time.h>

#include "dht22.h"

// SDA connected on RPi Plug P1 pin 7 (which is GPIO pin 4)
#define pin_SDA RPI_V2_GPIO_P1_07 

#define DHT_ERROR_TIMEOUT -1
#define DHT_ERROR_CHECKSUM -2
#define DHT_ERROR_ARGUMENT -3
#define DHT_ERROR_GPIO -4
#define DHT_SUCCESS 0

#define DHT_PULSES 41
#define DHT_MAXCOUNT 32000

void set_max_priority(void) {
  struct sched_param sched;
  memset(&sched, 0, sizeof(sched));
  // Use FIFO scheduler with highest priority for the lowest chance of the kernel context switching.
  sched.sched_priority = sched_get_priority_max(SCHED_FIFO);
  sched_setscheduler(0, SCHED_FIFO, &sched);
}

void set_default_priority(void) {
  struct sched_param sched;
  memset(&sched, 0, sizeof(sched));
  // Go back to default scheduler with default 0 priority.
  sched.sched_priority = 0;
  sched_setscheduler(0, SCHED_OTHER, &sched);
}


int pi_2_dht_read(float* humidity, float* temperature) {

  int i;
  // Validate humidity and temperature arguments and set them to zero.
  if (humidity == NULL || temperature == NULL) {
    return DHT_ERROR_ARGUMENT;
  }
  *temperature = 0.0f;
  *humidity = 0.0f;

  // Initialize GPIO library.
  //if (!bcm2835_init()){
  //  return DHT_ERROR_GPIO;
  //} 
	

  // Store the count that each DHT bit pulse is low and high.
  // Make sure array is initialized to start at zero.
  int pulseCounts[DHT_PULSES*2] = {0};

  // Set pin to output.
  ////pi_2_mmio_set_output(pin);
  bcm2835_gpio_fsel(pin_SDA, BCM2835_GPIO_FSEL_OUTP);
 
  // Bump up process priority and change scheduler to try to try to make process more 'real time'.
  set_max_priority();

  // Set pin high for ~500 milliseconds.
  ////pi_2_mmio_set_high(pin);
  bcm2835_gpio_write(pin_SDA, HIGH);
  bcm2835_delay(500);
  //sleep_milliseconds(500);

  // The next calls are timing critical and care should be taken
  // to ensure no unnecssary work is done below.

  // Set pin low for ~20 milliseconds.
  ////pi_2_mmio_set_low(pin);
  bcm2835_gpio_write(pin_SDA, LOW);
  bcm2835_delay(20);
  //busy_wait_milliseconds(20);

  // Set pin at input.
  ////pi_2_mmio_set_input(pin);
  bcm2835_gpio_fsel(pin_SDA, BCM2835_GPIO_FSEL_INPT); 	
 
  // Need a very short delay before reading pins or else value is sometimes still low.
  for (i=0; i < 40; ++i) {
  }

  // Wait for DHT to pull pin low.
  uint32_t count = 0;
  while (bcm2835_gpio_lev(pin_SDA))
 {
    if (++count >= DHT_MAXCOUNT) {
      // Timeout waiting for response.
      set_default_priority();
      return DHT_ERROR_TIMEOUT;
    }

  }

  // Record pulse widths for the expected result bits.
  for (i=0; i < DHT_PULSES*2; i+=2) {
    // Count how long pin is low and store in pulseCounts[i]
    while (!bcm2835_gpio_lev(pin_SDA)) {
      if (++pulseCounts[i] >= DHT_MAXCOUNT) {
        // Timeout waiting for response.
        set_default_priority();
        return DHT_ERROR_TIMEOUT;
      }

    }
    // Count how long pin is high and store in pulseCounts[i+1]
    while (bcm2835_gpio_lev(pin_SDA)) {
      if (++pulseCounts[i+1] >= DHT_MAXCOUNT) {
        // Timeout waiting for response.
        set_default_priority();
        return DHT_ERROR_TIMEOUT;
      }

    }
  }

  // Done with timing critical code, now interpret the results.

  // Drop back to normal priority.
  set_default_priority();

  // Compute the average low pulse width to use as a 50 microsecond reference threshold.
  // Ignore the first two readings because they are a constant 80 microsecond pulse.
  uint32_t threshold = 0;
  for (i=2; i < DHT_PULSES*2; i+=2) {
    threshold += pulseCounts[i];
  }
  threshold /= DHT_PULSES-1;

  // Interpret each high pulse as a 0 or 1 by comparing it to the 50us reference.
  // If the count is less than 50us it must be a ~28us 0 pulse, and if it's higher
  // then it must be a ~70us 1 pulse.
  uint8_t data[5] = {0};
  for (i=3; i < DHT_PULSES*2; i+=2) {
    int index = (i-3)/16;
    data[index] <<= 1;
    if (pulseCounts[i] >= threshold) {
      // One bit for long pulse.
      data[index] |= 1;
    }
    // Else zero bit for short pulse.
  }

  // Useful debug info:
  //printf("Data: 0x%x 0x%x 0x%x 0x%x 0x%x\n", data[0], data[1], data[2], data[3], data[4]);

  // Verify checksum of received data.
  if (data[4] == ((data[0] + data[1] + data[2] + data[3]) & 0xFF)) {
      // Calculate humidity and temp for DHT22 sensor.
      *humidity = (data[0] * 256 + data[1]) / 10.0f;
      *temperature = ((data[2] & 0x7F) * 256 + data[3]) / 10.0f;
      if (data[2] & 0x80) {
        *temperature *= -1.0f;
    }
    return DHT_SUCCESS;
  }
  else {
    return DHT_ERROR_CHECKSUM;
  }
}


