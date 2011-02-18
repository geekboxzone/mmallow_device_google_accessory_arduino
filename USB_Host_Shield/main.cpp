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
