/*
 * Copyright 2009-2011 Oleg Mazurov, Circuits At Home, http://www.circuitsathome.com
 * MAX3421E USB host controller support
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the authors nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
/* Mouse communication via control endpoint */
#include <spi.h>
#include <max3421e.h>
#include <usb.h>
 
#define DEVADDR 1
#define CONFVALUE 1
 
void setup();
void loop();
 
MAX3421E Max;
USB Usb;
 
void setup()
{
    Serial.begin( 115200 );
    Serial.println("Start");
    Max.powerOn();
    delay( 200 );
}
 
void loop()
{
 byte rcode;
    Max.Task();
    Usb.Task();
    if( Usb.getUsbTaskState() == USB_STATE_CONFIGURING ) {
        mouse0_init();
    }//if( Usb.getUsbTaskState() == USB_STATE_CONFIGURING...
    if( Usb.getUsbTaskState() == USB_STATE_RUNNING ) {  //poll the keyboard
        rcode = mouse0_poll();
        if( rcode ) {
          Serial.print("Mouse Poll Error: ");
          Serial.println( rcode, HEX );
        }//if( rcode...
    }//if( Usb.getUsbTaskState() == USB_STATE_RUNNING...
}
/* Initialize mouse */
void mouse0_init( void )
{
 byte rcode = 0;  //return code
  /**/
  Usb.setDevTableEntry( 1, Usb.getDevTableEntry( 0,0 ) );              //copy device 0 endpoint information to device 1
  /* Configure device */
  rcode = Usb.setConf( DEVADDR, 0, CONFVALUE );
  if( rcode ) {
    Serial.print("Error configuring mouse. Return code : ");
    Serial.println( rcode, HEX );
    while(1);  //stop
  }//if( rcode...
  Usb.setUsbTaskState( USB_STATE_RUNNING );
  return;
}
/* Poll mouse using Get Report and print result */
byte mouse0_poll( void )
{
  byte rcode,i;
  char buf[ 4 ] = { 0 };      //mouse buffer
  static char old_buf[ 4 ] = { 0 };  //last poll
    /* poll mouse */
    rcode = Usb.getReport( DEVADDR, 0, 4, 0, 1, 0, buf );
    if( rcode ) {  //error
      return( rcode );
    }
    for( i = 0; i < 4; i++) {  //check for new information
      if( buf[ i ] != old_buf[ i ] ) { //new info in buffer
        break;
      }
    }
    if( i == 4 ) {
      return( 0 );  //all bytes are the same
    }
    /* print buffer */
    if( buf[ 0 ] & 0x01 ) {
      Serial.print("Button1 pressed ");
    }
    if( buf[ 0 ] & 0x02 ) {
      Serial.print("Button2 pressed ");
    }
    if( buf[ 0 ] & 0x04 ) {
      Serial.print("Button3 pressed ");
    }
    Serial.println("");
    Serial.print("X-axis: ");
    Serial.println( buf[ 1 ], DEC);
    Serial.print("Y-axis: ");
    Serial.println( buf[ 2 ], DEC);
    Serial.print("Wheel: ");
    Serial.println( buf[ 3 ], DEC);
    for( i = 0; i < 4; i++ ) {
      old_buf[ i ] = buf[ i ];  //copy buffer
    }
    Serial.println("");
    return( rcode );
}
