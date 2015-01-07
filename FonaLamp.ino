#include <SoftwareSerial.h>
#include "Adafruit_FONA.h"
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <stdlib.h>
#include <avr/pgmspace.h>
#include <Adafruit_NeoPixel.h>
#define PIXEL_PIN 9
#define PIXEL_COUNT 1
#define PIXEL_TYPE NEO_KHZ800
#define FONA_RX 2
#define FONA_TX 3
#define FONA_RST 4
#define FONA_KEY 7
#define FONA_PS 8
#define led 13

Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_PIN, PIXEL_TYPE);
SoftwareSerial myfona = SoftwareSerial(FONA_TX, FONA_RX);
Adafruit_FONA fona = Adafruit_FONA(&myfona, FONA_RST);

int l; //loop counter

void setup() {
  Serial.begin(9600);
   strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  l = 1;
  pinMode(led, OUTPUT);
  digitalWrite(led, HIGH);
  delay(100);
  digitalWrite(led, LOW);
  delay(100);
  fona.setGPRSNetworkSettings(F("eseye.com"), F("user"), F("pass")); //your APN settings here
  pinMode(FONA_KEY, OUTPUT);
  //TurnOffFona();
  delay(1000);
  wdt_disable();
 }


void loop() {
  //wdt_reset();
  //Serial.println(F("starting loop"));
  digitalWrite(led, HIGH); 
  GetRGB();
  digitalWrite(led, LOW);
  l++;
  delay(100);
  TurnOffFona();
    //Serial.println(F("ending loop"));
  delay(30000);
sleepabit(3600);
//wdt_disable();
}

void GetRGB()
{
  //Serial.println(F("starting getRGB"));
  TurnOnFona();
  delay(1000);
  fona.begin(4800);                   
  delay(1000);
  //wdt_reset();
  GetConnected(); 
  fona.enableGPRS(false); 
  delay(3000);
  fona.enableGPRS(true);
  delay(3000);
  //Serial.println(F("gprs should be on"));
  delay(1000);

  strip.setPixelColor(0,atoi(getValue("XXXXvar1")),atoi(getValue("XXXXvar2")),atoi(getValue("XXXXvar3"))); //Variable IDs from Ubidots go here
  strip.show();
  
  GetDisconnected();
  //wdt_reset();  
  TurnOffFona(); 
  //wdt_reset(); 
}

char* getValue(char *variable){
   //Serial.println(F("starting getValue"));
   delay(1000);
   uint16_t statuscode;
   int16_t length;
   char url[150];
   sprintf(url,"%s%s%s","http://things.ubidots.com/api/v1.6/variables/",variable,"?token=XXXXXX"); //your Ubidots token goes here
   flushSerial();
   if (!fona.HTTP_GET_start(url, &statuscode, (uint16_t *)&length)) {
         Serial.println("Failed!");
       }

   char d[800];
   int i;
   i=0;  
   while (length > 0) {
         while (fona.available()) {
           d[i] = fona.read();    
           length--;
           i++;
           if(i > 799) break;
           if (! length) break;
         }
       }
   fona.HTTP_GET_end();  
 
   char *value;
   char *value2;
   char *pos;
   char delimiter[] = ":";
   char delimiter2[] = ".";
           char *ptr;
           char *ptr2;
           ptr = strtok(d, delimiter);
           int h = 0;
           while(ptr != NULL) {
             if(h==16)                //i know this is far from beautiful, but it worked for me that way
              {
                value=ptr;
                ptr2 = strtok(value,delimiter2);
                int h2 = 0;
                while(ptr2 != NULL) {
                  if(h2==0)
                    value2=ptr2;
                  ptr2 = strtok(NULL, delimiter2);
                  h2++;
                  }
              }
             ptr = strtok(NULL, delimiter);
             h++;
            }
  return value2;

}


void GetConnected()
{
  uint8_t n = 0;
  do
  {
    n = fona.getNetworkStatus();  // Read the Network / Cellular Status
    //Serial.print(F("Network status "));
    //Serial.print(n);
    //Serial.print(F(": "));
    if (n == 0);// Serial.println(F("Not registered"));
    if (n == 1);// Serial.println(F("Registered (home)"));
    if (n == 2);// Serial.println(F("Not registered (searching)"));
    if (n == 3);// Serial.println(F("Denied"));
    if (n == 4);// Serial.println(F("Unknown"));
    if (n == 5);// Serial.println(F("Registered roaming"));
    //wdt_reset();
  } 
  while (n != 5);
  //wdt_reset();
}

void GetDisconnected()
{
  fona.enableGPRS(false);
  //Serial.println(F("GPRS Serivces Stopped"));
}

void TurnOnFona()
{
  //Serial.println("Turning on Fona: ");
  while(digitalRead(FONA_PS)==LOW)
    {
    digitalWrite(FONA_KEY, LOW);
    }
    digitalWrite(FONA_KEY, HIGH);
}

 void TurnOffFona()
{
  //Serial.println("Turning off Fona ");
  while(digitalRead(FONA_PS)==HIGH)
  {
    digitalWrite(FONA_KEY, LOW);
    //delay(100);
    }
    digitalWrite(FONA_KEY, HIGH); 
}


void sleepabit(int howlong)
  {
  int i2 = 0;  
  //Serial.println("ok, trying to sleep for " + String(howlong*8) + " seconds");
  delay(100);  
  while (i2 < (howlong/8))
    {  
    cli();  
    delay(100); 
    // disable ADC
    //ADCSRA = 0;
    //prepare interrupts
    WDTCSR |= (1<<WDCE) | (1<<WDE);
    // Set Watchdog settings:
    WDTCSR = (1<<WDIE) | (1<<WDE) | (1<<WDP3) | (0<<WDP2) | (0<<WDP1) | (1<<WDP0);
    sei();
    wdt_reset();  
    set_sleep_mode (SLEEP_MODE_PWR_DOWN);  
    sleep_enable();
    // turn off brown-out enable in software
    MCUCR = bit (BODS) | bit (BODSE);
    MCUCR = bit (BODS); 
    sleep_cpu ();  
    // cancel sleep as a precaution
    sleep_disable();
    i2++;
    }
   wdt_disable(); 
  }
// watchdog interrupt
ISR (WDT_vect) 
{
  //i++;
   //Serial.println("waking up...");
}  // end of WDT_vect

void flushSerial() {
    while (Serial.available()) 
    Serial.read();
}

 
