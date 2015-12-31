/* main */
// RaspberryPI - DHT22 + BMP085 sensors main program - 2015.12.31 - @AM
// gcc -o dht22 -l rt dht22.c -l bcm2835 


// use: gcc -o main main.c bmp085.c dht22.c -lbcm2835 -lrt -lm

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <sched.h>
#include <string.h>
#include <time.h>
#include <bcm2835.h>

#include "bmp085.h"
#include "dht22.h"

#define DHT_ERROR_TIMEOUT -1
#define DHT_ERROR_CHECKSUM -2
#define DHT_ERROR_ARGUMENT -3
#define DHT_ERROR_GPIO -4
#define DHT_SUCCESS 0

int main(void)
{
	float humidity = 0.0, temperature = 0.0, thi = 0.0, temp = 0.0, pressure = 0.0, sealvl = 0.0;
    int j;
	char timestamp[20];
	char filename[40];
	char nomefile[40];
	char newfilename[40];
	time_t ltime;
	struct tm *tm;
	int minute;
	FILE *fp;

	// Initialize GPIO library.
	if (!bcm2835_init()){
		return DHT_ERROR_GPIO;
	} 

	ltime=time(NULL);
	tm=localtime(&ltime);
	sprintf(newfilename,"%04d-%02d-%02d.dat", tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday);

	while (1)
	{
		j=0;
		humidity = 0; 
		temperature = 0;
		
		strcpy(filename,newfilename);
		sprintf(nomefile,"/var/www/dat/%s",filename);
		fp = fopen(nomefile, "a");

		while (strcmp(newfilename,filename)==0)
		{
			//  Read humidity and temperature 
			while(pi_2_dht_read(&humidity, &temperature)!=0)
			{
				bcm2835_delay(2000);
				j+=1;
			}
			
			// Thermohygrometric Index
			thi = temperature - (0.55 - 0.0055 * humidity) * (temperature - 14.5);
			
			//printf("Lettura %d: humidity: %3.1f - temperature: %3.1f - thermohygrometric index: %3.1f\n", j, humidity, temperature, thi);
			

			//  Read temperature and pressure 
			if (readBmp085(BMP085_HIGH_RESOLUTION, &temp, &pressure)) 
			{
			/*
			* The pressure sensor is located approximately 150 meters over
			* sea level
			*/
				sealvl = convertToSeaLevel(pressure, 10.0);

			//printf("BMP085 Temp=%1.1f° Pressure %1.2fhPa Sea level %1.2fhPa\n", temp, pressure, sealvl);
			//} else {
			//printf("Can not access the BMP085\n");
			}
						
			
			ltime=time(NULL);
			tm=localtime(&ltime);

			sprintf(timestamp,"%d", ltime);
			sprintf(newfilename,"%04d-%02d-%02d.dat", tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday);

			if (minute != tm->tm_min)
			{
				minute = tm->tm_min;
				fprintf(fp,"%s;%3.1f;%3.1f;%3.1f;%3.1f;%3.1f;%3.1f\n", timestamp, humidity, temperature, thi, temp, pressure, sealvl);
				fflush (fp);
			}

			// Wait 5 second (5000 ms) before reading next temperature
			delay(5000);
		}

		fclose(fp);
	}
	return (0);

	

}
