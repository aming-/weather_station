/**
 * This file is part of Weather Sensor Reading Library.
 *
 * Copyright (C) 2014 Ulrich Eckhardt
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
 *
 */

#ifndef _BMP085_H
#define _BMP085_H

#include <stdbool.h>

typedef enum {
	BMP085_ULTRALOWPOWER 	= 0,
	BMP085_STANDARD		= 1,
	BMP085_HIGH_RESOLUTION	= 2,
	BMP085_ULTRA_HIGH_RESOLUTION = 3
} BMP085_OSS; // Oversampling settings

bool readBmp085(BMP085_OSS oss, float *temp, float *pressure);
float convertToSeaLevel (float pressure, float alt);

#endif
