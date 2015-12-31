/**
 * This file is part of Weather Sensor Reading Library.
 *
 * Copyright (C) 2014-2015 Ulrich Eckhardt
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.

 Circuit detail:
	Using a Spark Fun Barometric Pressure Sensor - BMP085 breakout board
	link: https://www.sparkfun.com/products/9694
	This comes with pull up resistors already on the i2c lines.
	BMP085 pins below are as marked on the Sparkfun BMP085 Breakout board

	SDA	- 	P1-03 / IC20-SDA
	SCL	- 	P1-05 / IC20_SCL
	XCLR	- 	Not Connected
	EOC	-	Not Connected
	GND	-	P1-06 / GND
	VCC	- 	P1-01 / 3.3V
	
	Note: Make sure you use P1-01 / 3.3V NOT the 5V pin.
*/


#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <math.h>
#include <stdint.h>
#include <inttypes.h>
#include <unistd.h>

#include <bcm2835.h>

#include "bmp085.h"

#define BMP085_DEVID		0x77

#define BMP085_CALIBSTART	0xAA
#define BMP085_WRITE		0xF4
#define BMP085_READ		    0xF6

#define BMP085_READ_TEMPRAW 	0x2E
#define BMP085_READ_PRESSURERAW	0x34

static bcm2835I2CReasonCodes i2cErr = BCM2835_I2C_REASON_OK;

/* Name of the BMP85 clibration registers */
typedef enum _bmp85_calib {
	BMP85_AC1, BMP85_AC2, BMP85_AC3, BMP85_AC4, BMP85_AC5, BMP85_AC6,
	BMP85_B1, BMP85_B2,
	BMP85_MB, BMP85_MC, BMP85_MD,
	BMP85_LAST
} BMP85_CALIB_T;

/* Value of the calibration registers */
static int32_t bmp085calib_reg[BMP85_LAST];

/* Timeout in ms */
static int bmp085_pressureconv[BMP085_ULTRA_HIGH_RESOLUTION+1] = {
	5, 8, 14, 26
};

static uint8_t i2cRead(uint8_t reg)
{
    uint8_t buf = 0;
    i2cErr = bcm2835_i2c_read_register_rs((char *)&reg, (char *)&buf, 1);
    return buf;
}

static bool i2cWrite(uint8_t reg, uint8_t val)
{
    uint8_t buf[2];
    buf[0] = reg;
    buf[1] = val;
    i2cErr = bcm2835_i2c_write((char *)buf, 2);
    if (i2cErr!= BCM2835_I2C_REASON_OK) {
        return false;
    }
    return true;
}

/*
 * Read a signed 16 bit value from a register via I2C bus and return it
 * as 32bit value
 */
static bool readSigned(int reg, int32_t *val)
{
    int16_t v1;
    char buf[2];
    *val = 0;
    i2cErr = bcm2835_i2c_read_register_rs((char *) &reg, buf, 2);
    if (i2cErr!= BCM2835_I2C_REASON_OK) {
        return false;
    }
    /* Get high byte */
    v1 = ((int16_t) buf[0] & 0xFF) << 8;
    /* Get low byte */
    v1 |= ((int16_t) buf[1] & 0xFF);
    *val = v1;
    return true;
}

/*
 * Read an unsigned 16 bit value from a register via I2C bus and return it
 * as 32bit value
 */
static bool readUnsigned(int reg, int32_t *val)
{
    char buf[2];
    uint16_t v1;
    *val = 0;

    i2cErr = bcm2835_i2c_read_register_rs((char *) &reg, buf, 2);
    if (i2cErr != BCM2835_I2C_REASON_OK) {
        return false;
    }
    /* Get high byte */
    v1 = ((uint16_t) buf[0] & 0xFF) << 8;
    /* Get low byte */
    v1 |= ((uint16_t) buf[1] & 0xFF);
    *val = v1;
    return (true);
}

/*
 * Read temperature and pressure from bmp085
 */
bool readBmp085(BMP085_OSS oss, float *temp, float *pressure)
{
    BMP85_CALIB_T calib;
    int reg = 0xAA;
    int32_t tempraw = 0;
    int32_t t;
    int32_t msb, lsb, xlsb;
    int64_t x1, x2, x3;
    int64_t b3, b5, b6;
    uint64_t b4;
    uint64_t b7;
    int64_t up = 0;
    int64_t p = 0;

    /*
     * Setup I2C bus
     */
//    if (!bcm2835_init()) {
//        fprintf(stderr, "bcm2835_init failed: %s\n", strerror(errno));
//        return false;
//    }

    bcm2835_i2c_begin();
    bcm2835_i2c_setSlaveAddress(BMP085_DEVID);

    /*
     * Read calibration values
     */
    for (calib = BMP85_AC1; calib < BMP85_LAST; calib++) {
        if ((calib >= BMP85_AC4) && (calib <= BMP85_AC6)) {
            if (!readUnsigned(reg, &bmp085calib_reg[calib])) {
                fprintf(stderr, "Read calibration register %02x failed: %d\n",
                        reg, i2cErr);
                return false;
            }
        } else {
            if (!readSigned(reg, &bmp085calib_reg[calib])) {
                fprintf(stderr, "Read calibration register %02x failed: %d\n",
                        reg, i2cErr);
                return false;
            }
        }
#ifdef DEBUG
        printf("Reg %02x = %"PRId32"\n", reg, bmp085calib_reg[calib]);
#endif
        reg += 2;
    }

    /*
     * Read raw temperature
     */

    if (!i2cWrite(BMP085_WRITE, BMP085_READ_TEMPRAW)) {
        fprintf(stderr, "Read raw temperature failed: %d\n", i2cErr);
        return false;
    }
    bcm2835_delay(6);
    if (!readUnsigned(BMP085_READ, &tempraw)) {
        fprintf(stderr, "Read raw temperature failed: %d\n", i2cErr);
        return false;
    }
#ifdef DEBUG
    printf ("RawTemp %d\n", tempraw);
#endif
    /*
     * Calculate real temperature
     */
    x1 = (tempraw - bmp085calib_reg[BMP85_AC6]) * bmp085calib_reg[BMP85_AC5]
            >> 15;
    x2 = ((int64_t) bmp085calib_reg[BMP85_MC] << 11)
            / (x1 + bmp085calib_reg[BMP85_MD]);
    b5 = x1 + x2;
    t = (b5 + 8) >> 4;
    *temp = (float) t / 10.0;

    /*
     * Start reading the raw pressure
     */

    if (!i2cWrite(BMP085_WRITE, BMP085_READ_PRESSURERAW + (oss << 6))) {
        fprintf(stderr, "Read raw pressure failed: %d\n", i2cErr);
        return false;
    }
    /*
     * Wait until bmp085 has finished reading the pressure
     */
    bcm2835_delay(bmp085_pressureconv[oss]);

    /*
     * Read msb, lsb and xlsb
     */
    msb = i2cRead(BMP085_READ);
    lsb = i2cRead(BMP085_READ + 1);
    xlsb = i2cRead(BMP085_READ + 2);

    up = ((msb << 16) | (lsb << 8) | xlsb) >> (8 - oss);
#ifdef DEBUG
    printf ("Raw pressure %"PRId64" %"PRId32" %"PRId32" %"PRId32"\n",
            up, msb, lsb, xlsb);
#endif
    /*
     * Calculate pressure
     */
    b6 = b5 - 4000;
    x1 = ((int64_t) bmp085calib_reg[BMP85_B2] * ((b6 * b6) >> 12)) >> 11;
    x2 = ((int64_t) bmp085calib_reg[BMP85_AC2] * b6) >> 11;
    x3 = x1 + x2;

    b3 = (((((int64_t) bmp085calib_reg[BMP85_AC1] << 2) + x3) << oss) + 2) >> 2;

    x1 = ((int64_t) bmp085calib_reg[BMP85_AC3] * b6) >> 13;
    x2 = ((int64_t) bmp085calib_reg[BMP85_B1] * ((b6 * b6) >> 12)) >> 16;
    x3 = (x1 + x2 + 2) >> 2;

    b4 = (bmp085calib_reg[BMP85_AC4] * (uint64_t) (x3 + 32768)) >> 15;
    b7 = (uint64_t) (up - b3) * (50000 >> oss);
    if (b7 < 0x80000000) {
        p = (b7 << 1) / b4;
    } else {
        p = (b7 / b4) << 1;
    }
    x1 = (p >> 8) * (p >> 8);
    x1 = (x1 * 3038) >> 16;
    x2 = (-7357 * p) >> 16;
    p += (x1 + x2 + 3791) >> 4;

    *pressure = (float) p / 100.0;
    bcm2835_i2c_end();
    return true;
}

/*
 * Convert the pressure returned by the sensor to the pressure at
 * sea level. Alt is the altitude above sea level in meters of the sensor
 * location.
 */
float convertToSeaLevel(float pressure, float alt)
{
    return pressure / powf((1.0 - (alt / 44330.0)), 5.255);
}



