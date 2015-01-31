/*
 * SD2405RTC.h - library for i2c SD2405 Real Time Clock
 *
 * Copyright (c) Netgrowing 2015
 * Written by Julien Gautier for Netgrowing Electronics.
 * 
 * This library is intended to be uses with Arduino Time.h library functions
 * 
 * The library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 * 
 * 2015/01/30 - Initial release
 */

#include "Arduino.h"
#include <Wire.h>
#include "SD2405RTC.h"

#define SD2405_ADDR 0x32

SD2405RTC::SD2405RTC()
{
  Wire.begin();
}
  
// PUBLIC FUNCTIONS

time_t SD2405RTC::get()   // Aquire data from buffer and convert to time_t
{
  tmElements_t tm;
  read(tm);
  return(makeTime(tm));
}

void  SD2405RTC::set(time_t t)
{
  tmElements_t tm;
  breakTime(t, tm);
  write(tm); 
}

// Aquire datetime data from the RTC chip in BCD format
void SD2405RTC::read( tmElements_t &tm)
{
  unsigned char date[7];
  unsigned char n=0;
  unsigned char i;
 
  Wire.requestFrom(SD2405_ADDR,tmNbrFields);
  while(Wire.available())
  {  
    date[n++]=Wire.read();
  }
  delayMicroseconds(1);
  Wire.endTransmission();
  for(i=0;i<7;i++)
  {
    if(i==2) {
      date[i]=(date[i]&0x7f); //clear the hour's highest bit 12_/24 ; 0x7F is bcd 0111-1111
    }
    //date[i]=(((date[i]&0xf0)>>4)*10)+(date[i]&0x0f); // ; 0xF0 is bcd 1111-0000 ; 0x0f is bcd 0000-1111
    date[i]=bcd2dec(date[i]);
  }
  tm.Second = date[0];
  tm.Minute = date[1];
  tm.Hour = date[2];
  tm.Wday = date[3];
  tm.Day = date[4];
  tm.Month = date[5];
  tm.Year = y2kYearToTm(date[6]);
}

// Write datetime data to the RTC chip in BDC format
void SD2405RTC::write(tmElements_t &tm)
{
  writeTimeOn();
  Wire.beginTransmission(SD2405_ADDR);
  Wire.write(byte(0));                  // reset register pointer  
  Wire.write(dec2bcd(tm.Second)) ;   
  Wire.write(dec2bcd(tm.Minute));
  Wire.write(dec2bcd(tm.Hour+80));      // +80: sets 24 hours format
  Wire.write(dec2bcd(tm.Wday));   
  Wire.write(dec2bcd(tm.Day));
  Wire.write(dec2bcd(tm.Month));
  Wire.write(dec2bcd(tm.Year));
  Wire.endTransmission();
  Wire.beginTransmission(SD2405_ADDR);
  Wire.write(0x12);                     // set the address for writing as 12H : Time Trimming Register
  Wire.write(byte(0));                  // Counts will not change when (F6, F5, F4, F3, F2, F1, F0) are set to (*, 0, 0, 0, 0, 0, *).
  Wire.endTransmission();
  writeTimeOff();
}

// PRIVATE FUNCTIONS

// Convert Decimal to Binary Coded Decimal (BCD)
uint8_t SD2405RTC::dec2bcd(uint8_t num)
{
  return ((num/10 * 16) + (num % 10));
}

// Convert Binary Coded Decimal (BCD) to Decimal
uint8_t SD2405RTC::bcd2dec(uint8_t num)
{
  return ((num/16 * 10) + (num % 16));
}
 
//Enable writing to SD2405
void SD2405RTC::writeTimeOn(void)
{       
  Wire.beginTransmission(SD2405_ADDR);
  Wire.write(0x10);       //Set the address for writing as 10H
  Wire.write(0x80);       //Set WRTC1=1
  Wire.endTransmission();
  Wire.beginTransmission(SD2405_ADDR);    
  Wire.write(0x0F);       //Set the address for writing as OFH
  Wire.write(0x84);       //Set WRTC2=1,WRTC3=1      
  Wire.endTransmission();
}
 
//Disable writing to SD2405
void SD2405RTC::writeTimeOff(void)
{       
  Wire.beginTransmission(SD2405_ADDR);   
  Wire.write(0x0F);       //Set the address for writing as OFH          
  Wire.write(byte(0));    //Set WRTC2=0,WRTC3=0
  Wire.write(byte(0));    //Set WRTC1=0  
  Wire.endTransmission();
}
// create an instance for the user
SD2405RTC RTC = SD2405RTC();

