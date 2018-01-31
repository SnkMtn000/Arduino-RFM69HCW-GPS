/*
 * A simple GPS Feld Hell Telemetry sender
 * for the HopeRF rfm69hcw
 * on a ProMini 3.3
 * Charles Webb
 * KC1ENN@arrl.net
 * 
 * with thanks to James Coxon
 * 
 * Feld Hell for HopeRF rfm69hcw modifed from source below
 * https://github.com/jamescoxon/PicoAtlas/blob/master/Pico6MK3/sketch_apr02b/sketch_apr02b.ino
 * 
 * 
 */

#include <SPI.h>
#include <TinyGPS++.h>
#include <SoftwareSerial.h>

#define MY_DELAY 5000  // delay in loop, between sending the message
//#define POWER 0x50 // -2 dBm
///#define POWER 0x52  // 0 dBm
//#define POWER 0x55  //  3 dBm
//#define POWER 0x58  // 6 dBm
//#define POWER 0x5B  //  9 dBm
//#define POWER 0x5E  // 12 dBm
#define POWER 0x7F  // 17 dBm
#define ssPin 10  //  SS
#define rstPin 9 //  reset pin
#define MODULATION 0x48 //  Modulation for OOK, Continuous w.o bit synch, OOK, no shaping
#define myFrequency 433200000
//#define myFrequency 915000000

unsigned int count = 0;
char mytext[80];
String myCallSign = "CALLSIGN/B";
String myString = "";
int myHour, myMinute, mySecond, myAlt, mySpd, mySat;
double myLat, myLng;

TinyGPSPlus gps;
SoftwareSerial ss(6,7);  // Tx Rx

void setup(){
  Serial.begin(9600);
  ss.begin(9600);
  pinMode(ssPin, OUTPUT);
  pinMode(rstPin, OUTPUT);
  setupSPI();
  delay(1000);
  resetRFM69();
  delay(1000);
  setupRFM69();
}
 
void loop(){
  count++;
  fetchGPS();
  buildString();
  myString.toCharArray(mytext,myString.length() + 1);
  hellsendmsg(mytext);
  delay(MY_DELAY);
}

void buildString() {
  myString = "...";
  myString += myCallSign;
  myString += " COUNT " + String(count);
  myString += " TIME " + String(myHour) + "/" + String(myMinute) + "/" + String(mySecond);
  myString += " LAT " + String(myLat,7) + " LNG " + String(myLng,7);
  myString += " ALT " + String(myAlt) + " SPD " + String(mySpd) + " SAT " + String(mySat);
  myString += "...";
  Serial.print("I am sending: "); Serial.println(myString);
}

void fetchGPS() {  //Remeber to only enable and softwear serial as needed, to thre the GPS then disable as it messes with the timing/printing of FH
  myHour = gps.time.hour(); 
  myMinute = gps.time.minute(); 
  mySecond = gps.time.second(); 
  myLat = gps.location.lat(); 
  myLng = gps.location.lng();
  myAlt = gps.altitude.feet();
  mySpd = gps.speed.mph();
  mySat = gps.satellites.value();
  smartDelay(1000);
}

void setupRFM69() {
  writeReg(0x01,0x0C);
  writeReg(0x02,MODULATION);  
  setFrequency(myFrequency);
  Serial.print("Frequency set to "); Serial.println(myFrequency);
  writeReg(0x26,0x07); // CLK off to save power
}

// set the frequency (in Hz)
void setFrequency(uint32_t freqHz){
  freqHz /= 61; // divide down by FSTEP to get FRF
  writeReg(0x07, freqHz >> 16);
  writeReg(0x08, freqHz >> 8);
  writeReg(0x09, freqHz);
}

void resetRFM69() {
  digitalWrite(rstPin, HIGH);
  delay(100);
  digitalWrite(rstPin, LOW);
  delay(100);
}

void printReg(byte data) {
  Serial.print("Register "); 
  Serial.print(data); 
  Serial.print(" = "); 
  Serial.println(readReg(data), HEX);
}

void writeReg(uint8_t addr, uint8_t value) {
  digitalWrite(ssPin,LOW);
  SPI.transfer(addr | 0x80);
  SPI.transfer(value);
  digitalWrite(ssPin, HIGH);
}

uint8_t readReg(uint8_t addr) {
  digitalWrite(ssPin, LOW);
  SPI.transfer(addr & 0x7F);
  uint8_t regval = SPI.transfer(0);
  digitalWrite(ssPin, HIGH);
  return regval;
}

void setupSPI() {
  SPI.begin();
  SPI.setBitOrder(MSBFIRST);
  SPI.setClockDivider(SPI_CLOCK_DIV4);
}

//***************Hellschreiber******************
 
struct t_htab { char c; int hellpat[5]; } ;
 
struct t_htab helltab[] = {
 
  {'1', { B00000100, B00000100, B01111100, B00000000, B00000000 } },
  {'2', { B01001000, B01100100, B01010100, B01001100, B01000000 } },
  {'3', { B01000100, B01000100, B01010100, B01010100, B00111100 } },
  {'4', { B00011100, B00010000, B00010000, B01111100, B00010000 } },
  {'5', { B01000000, B01011100, B01010100, B01010100, B00110100 } },
  {'6', { B00111100, B01010010, B01001010, B01001000, B00110000 } },
  {'7', { B01000100, B00100100, B00010100, B00001100, B00000100 } },
  {'8', { B00111100, B01001010, B01001010, B01001010, B00111100 } },
  {'9', { B00001100, B01001010, B01001010, B00101010, B00111100 } },
  {'0', { B00111000, B01100100, B01010100, B01001100, B00111000 } },
  {'A', { B01111000, B00101100, B00100100, B00101100, B01111000 } },
  {'B', { B01000100, B01111100, B01010100, B01010100, B00101000 } },
  {'C', { B00111000, B01101100, B01000100, B01000100, B00101000 } },
  {'D', { B01000100, B01111100, B01000100, B01000100, B00111000 } },
  {'E', { B01111100, B01010100, B01010100, B01000100, B01000100 } },
  {'F', { B01111100, B00010100, B00010100, B00000100, B00000100 } },
  {'G', { B00111000, B01101100, B01000100, B01010100, B00110100 } },
  {'H', { B01111100, B00010000, B00010000, B00010000, B01111100 } },
  {'I', { B00000000, B01000100, B01111100, B01000100, B00000000 } },
  {'J', { B01100000, B01000000, B01000000, B01000000, B01111100 } },
  {'K', { B01111100, B00010000, B00111000, B00101000, B01000100 } },
  {'L', { B01111100, B01000000, B01000000, B01000000, B01000000 } },
  {'M', { B01111100, B00001000, B00010000, B00001000, B01111100 } },
  {'N', { B01111100, B00000100, B00001000, B00010000, B01111100 } },
  {'O', { B00111000, B01000100, B01000100, B01000100, B00111000 } },
  {'P', { B01000100, B01111100, B01010100, B00010100, B00011000 } },
  {'Q', { B00111000, B01000100, B01100100, B11000100, B10111000 } },
  {'R', { B01111100, B00010100, B00010100, B00110100, B01011000 } },
  {'S', { B01011000, B01010100, B01010100, B01010100, B00100100 } },
  {'T', { B00000100, B00000100, B01111100, B00000100, B00000100 } },
  {'U', { B01111100, B01000000, B01000000, B01000000, B01111100 } },
  {'V', { B01111100, B00100000, B00010000, B00001000, B00000100 } },
  {'W', { B01111100, B01100000, B01111100, B01000000, B01111100 } },
  {'X', { B01000100, B00101000, B00010000, B00101000, B01000100 } },
  {'Y', { B00000100, B00001000, B01110000, B00001000, B00000100 } },
  {'Z', { B01000100, B01100100, B01010100, B01001100, B01100100 } },
  {'.', { B01000000, B01000000, B00000000, B00000000, B00000000 } },
  {',', { B10000000, B10100000, B01100000, B00000000, B00000000 } },
  {'/', { B01000000, B00100000, B00010000, B00001000, B00000100 } },
  {'*', { B00000000, B00000000, B00000100, B00001110, B00000100 } }
 
};
 
#define N_HELL  (sizeof(helltab)/sizeof(helltab[0]))
 
void helldelay() {
  //Slow Hell
  //delay(64);
  //delayMicroseconds(900);
 
  //Feld-Hell
  delay(8);  //  was 8 and hands off
  delayMicroseconds(100);  // was 160  used to adjust slope of displayed text
}
 
void on() {
  writeReg(0x11,radioPower); // PA On
  helldelay();
  writeReg(0x11,0x00);
}

void hellsend(char c) {
  int i ;
  if (c == ' ') {
      for (int d=0; d<14; d++){
        helldelay();  
      }
    return ;
  }
  for (i=0; i<N_HELL; i++) {
    if (helltab[i].c == c) {
      //Serial.print(helltab[i].c) ;
 
      for (int j=0; j<=4; j++) 
      {
        byte mask = B10000000;
        for (int q=0; q<=6; q++)
        {      
          if(helltab[i].hellpat[j] & mask) {
            on();
          } else {
            helldelay();
          }
          mask >>= 1;
        }
      }
      for (int d=0; d<14; d++){
        helldelay();  
      }
      return ;
    }
  }
  /* if we drop off the end, then we send a space */
  //Serial.print("?") ;
}
 
void hellsendmsg(char *str) {
  //flag = 1;
  delay(1000);
  while (*str)
    hellsend(*str++) ;
  //Serial.println("");
  //flag = 0;
}

// This custom version of delay() ensures that the gps object is being "fed".
static void smartDelay(unsigned long ms)
{
  unsigned long start = millis();
  do 
  {
    while (ss.available())
      gps.encode(ss.read());
  } while (millis() - start < ms);
}

