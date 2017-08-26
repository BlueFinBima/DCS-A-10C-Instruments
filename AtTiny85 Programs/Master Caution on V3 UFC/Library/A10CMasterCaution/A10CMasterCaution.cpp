/*
  A10CMasterCaution.cpp - I2C ATtiny85 used for Master Caution LED on the UFC of A-10C
  Revision 1.1
  Copyright (c) 2015 Neil Larmour.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/
#include "A10CMasterCaution.h"

#if (ARDUINO >= 100)
#include <Arduino.h>
#else
#include <WProgram.h>
#endif

#include <Wire.h>
#include <inttypes.h>

A10CMASTERCAUTION::A10CMASTERCAUTION(uint8_t address)
{
    deviceaddress = address;
    //Wire.begin(); 
}

bool A10CMASTERCAUTION::setMasterCautionLED()
{
  Wire.beginTransmission(deviceaddress);
  Wire.write(SETMCLED);
  Wire.endTransmission();
  return true;
}
bool A10CMASTERCAUTION::setLED(uint8_t lednum)
{
  Wire.beginTransmission(deviceaddress);
  Wire.write(SETLED);
  Wire.write(lednum);
  Wire.endTransmission();
  delay(10);
  return true;
}
bool A10CMASTERCAUTION::resetMasterCautionLED()
{
  Wire.beginTransmission(deviceaddress);
  Wire.write(RESETMCLED);
  Wire.endTransmission();
  return true;
}
bool A10CMASTERCAUTION::resetLED(uint8_t lednum)
{
  Wire.beginTransmission(deviceaddress);
  Wire.write(RESETLED);
  Wire.write(lednum);
  Wire.endTransmission();
  delay(10);
  return true;
}

bool A10CMASTERCAUTION::setAddress(uint8_t newaddress)
{
  Wire.beginTransmission(deviceaddress);
  Wire.write(SETADDRESSREG);
  Wire.write(newaddress);
  Wire.endTransmission();
  deviceaddress = newaddress;
  delay(10);
  return true;
} 


