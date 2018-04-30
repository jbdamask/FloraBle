#include <Arduino.h>
#include <SPI.h>
#include "Adafruit_BLE.h"
#include "Adafruit_BluefruitLE_SPI.h"
#include "Adafruit_BluefruitLE_UART.h"
#include "BluefruitConfig.h"
Adafruit_BluefruitLE_UART ble(BLUEFRUIT_HWSERIAL_NAME, BLUEFRUIT_UART_MODE_PIN);
/*=========================================================================
    APPLICATION SETTINGS

    FACTORYRESET_ENABLE       Perform a factory reset when running this sketch
    NUMPIXELS                 How many NeoPixels are attached to the Arduino?
    -----------------------------------------------------------------------*/
    #define FACTORYRESET_ENABLE     1
    /* Rename device */
    #define DEVICE_NAME             "AT+GAPDEVNAME=TouchLightsBle"
    /*----------------------- 
       BUTTON
       Put a 2.2K resistor between data leg and 3.3v to reduce noise
     -----------------------*/
    int buttonPin = 10;    
    // Keeps track of the last pins touched
    // so we know when buttons are 'released'
    int buttonState;
    int lastButtonState = LOW;
    
    /* We'll cycle through event states on button pushes */
    uint8_t lastState = 0;
    uint8_t currentState = 1;
    uint8_t minState = 0;
    uint8_t maxState = 11;
    // the packet buffer
    extern uint8_t packetbuffer[];
    // the ble payload, set to max buffer size
    uint8_t payload[21];
    // the defined length of our payload
    int colorLength = 4;
    uint16_t colLen = 3;    
    // Payload stuff
    uint8_t xsum = 0;
    // Button debouncing
    // the following variables are unsigned longs because the time, measured in
    // milliseconds, will quickly become a bigger number than can be stored in an int.
    unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
    unsigned long debounceDelay = 10;    // the debounce time; increase if the output flickers    
/*=========================================================================*/    

/* Helper function for printing errors */
void error(const __FlashStringHelper*err) {
  Serial.println(err);
  while (1);
}


void setup()
{
//  while (!Serial);  // Uncomment when connected to computer. Comment out otherwise
  delay(500);
  Serial.begin(9600);
  /* Initialise the module */
  Serial.print(F("Initialising the Bluefruit LE module: "));
  if ( !ble.begin(VERBOSE_MODE) )
  {
    error(F("Couldn't find Bluefruit, make sure it's in CoMmanD mode & check wiring?"));
  }
  Serial.println( F("OK!") );
  
  if ( FACTORYRESET_ENABLE )
  {
    /* Perform a factory reset to make sure everything is in a known state */
    Serial.println(F("Performing a factory reset: "));
    if ( ! ble.factoryReset() ){
      error(F("Couldn't factory reset"));
    }
  }
  pinMode(buttonPin, INPUT); // Set button

  /* Customize name */
  ble.println(DEVICE_NAME);
  /* Initialise the module */
  Serial.print(F("Initialising the Bluefruit LE module: "));
 
  /* Disable command echo from Bluefruit */
  ble.echo(false);
  Serial.println("Requesting Bluefruit info:");
  /* Print Bluefruit information */
  ble.info();
  ble.verbose(false);  // debug info is a little annoying after this point!
  ble.setMode(BLUEFRUIT_MODE_DATA);
  Serial.println("All set, let's go!");
}

void loop()
{
  int reading = digitalRead(buttonPin);
  if(reading != lastButtonState){
    // reset the debouncing timer
    lastDebounceTime = millis();
  }
  
  if ((millis() - lastDebounceTime) > debounceDelay) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:
    // if the button state has changed:
    if(reading != buttonState){
      buttonState = reading;
      if(buttonState == LOW){
        if (currentState < maxState)
        {
          currentState = currentState + 1;
        }else{
          currentState = minState;
        }
        Serial.print("Button pressed! New state is: ");
        Serial.println(currentState);
        packAndSend(); 
      }
    }
  }
  lastButtonState = reading;
}

void packAndSend()
{
  Serial.println("Sending data!");
  // Now package into a packetbuffer and write to Bluetooth
  payload[0] = 0x21;
  payload[1] = 0x42;
  payload[2] = currentState;

  xsum = 0;
  //for (uint8_t i=0; i<colLen; i++) {
  for (uint8_t i=0; i<colorLength-1; i++) {
    xsum += payload[i];
  }
  xsum = ~xsum;    
  payload[3] = xsum;  
  for(int i = 0; i < 4; i++){
    Serial.println(payload[i]);
  }
  ble.write(payload,colorLength);
}

