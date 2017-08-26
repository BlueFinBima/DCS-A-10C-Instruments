/*
  A10CLandingLightsandSkid.h - I2C switch used for Landing Gear on A-10C
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
#include "A10CLandingLightsandSkid.h"

#if (ARDUINO >= 100)
#include <Arduino.h>
#else
#include <WProgram.h>
#endif

#include <Wire.h>
#include <inttypes.h>

A10CLandingLightsandSkid::A10CLandingLightsandSkid(uint8_t address)
{
    deviceaddress = address;
    Wire.begin(); 
}

uint8_t A10CLandingLightsandSkid::readSwitch()
{
  Wire.beginTransmission(deviceaddress);
  Wire.write(READSWITCH);
  Wire.endTransmission();

  Wire.requestFrom(deviceaddress, (uint8_t)1);
  int retry = 0;
  while((Wire.available() < 1) && (retry < RETRYCOUNT))
    retry++;
  
  if (Wire.available() >= 1)  
  {
    uint8_t a = Wire.read();
    return a; 
  } else return false;
}


bool A10CLandingLightsandSkid::setAddress(uint8_t newaddress)
{
  Wire.beginTransmission(deviceaddress);
  Wire.write(SETADDRESSREG);
  Wire.write(newaddress);
  Wire.endTransmission();
  deviceaddress = newaddress;
  delay(10);
  return true;
} 


