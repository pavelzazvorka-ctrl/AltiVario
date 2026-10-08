/* 
  KDR Vario. Digital variometer based on Arduino.
  Copyright (C) 2011 Andrey Lebedev

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <math.h>
#include "atmosphere.h"

const float stdpressure = 1013.25;

Atmosphere::Atmosphere() {
	_zeropressure = stdpressure;
}

void Atmosphere::setZeroPressure(float _altitude, float _pressure, float _temperature) {
  float zp = _altitude * 0.0065 / (_temperature + 273.15);
  zp = zp + 1;
  zp = pow ( zp, 5.257);
  zp = zp * _pressure;
  _zeropressure = zp;
}

float Atmosphere::getAltitude(float pressure, float temperature) {
  return ((pow((_zeropressure / pressure), 1/5.257) - 1.0) * (temperature + 273.15)) / 0.0065;
}

float Atmosphere::getZeroPressure() {
	return _zeropressure;
}

void Atmosphere::setZeroPressure(float zeropressure) {
	_zeropressure = zeropressure;
}

