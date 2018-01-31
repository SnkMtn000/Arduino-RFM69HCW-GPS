/*
 * A simple GPS Morse Beacon Sender
 * for the HopeRF rfm69hcw
 * on a ProMini 3.3
 * Charles Webb
 * KC1ENN@arrl.net
 * 
 * With thanks to....
 * 
 * "This code is so trivial that I'm releasing it completely without restrictions."
 * Simple Arduino Morse Beacon v0.0
 * Written by Mark VandeWettering <k6hx@arrl.net>
 * Check out my blog @ http://brainwagon.org
 */
 
#include <SPI.h>

#define ssPin 10 // SS  Chip select pin for reading/writing to the rf69 registers with SPI
//#define ssPin 8  // adafruit feather M0
#define rstPin 9 //  reset pin
//#define rstPin 4 // adafruit feather M0
#define MY_DELAY 3000  // 3 second delay between loops, sending the message
#define N_MORSE (sizeof(morsetab)/sizeof(morsetab[0]))
#define SPEED (15) // WORKS 5 - 50 wpm FL-DIGI
#define DOTLEN (1200/SPEED)
#define DASHLEN (3*(1200/SPEED))
//#define MY_TX_POWER 0x50  // PA0 only range 0x50 -18dBm to 0x5F -3dBm
//#define MY_TX_POWER 0x58  // 6 dBm
//#define MY_TX_POWER 0x5B  //  9 dBm
//#define MY_TX_POWER 0x5E  // 12 dBm
//#define MY_TX_POWER 0x70 //  2 dbm
#define MY_TX_POWER 0x7F  // 17 dBm
#define MY_MODULATION 0x48 //  Modulation for OOK, Continuous w.o bit synch, OOK, no shaping

char mytext[80];
String inputString = "";         // a String to hold incoming data
String myCallSign = "CALLSIGN/B TEST BEACON"; //  Put more than callsign here, put your beacon string here
long myFrequency = 433200000;
//long myFrequency = 915000000;
String myString = "";
String myCall = ". CALLSIGN/B";
int myStringLen = 0;
unsigned int count=0;
//char mycount = 0;
int myHour, myMinute, mySecond, myAlt, mySpd, mySat;
double myLat, myLng;

void setup() {
  Serial.begin(9600);   // baud speed for sending to the arduino serial monitor
  Serial.println("Starting up");  // print to the serial monitor
  pinMode(ssPin, OUTPUT);  // define
  pinMode(rstPin, OUTPUT);
  setupSPI();
  delay(1000);
  resetRFM69();
  delay(1000);
  setupRFM69();
}

void loop() {
  count++;
  buildString();
  //myString = myCall;
        myString.toUpperCase();
        myStringLen = myString.length();
        myString.toCharArray(mytext,myString.length() + 1);
     //   Serial.print("I am sending: "); Serial.println(myString);
       sendmsg(mytext);
  inputString = "";
  myString = "";
}

void buildString() {
  myString = myCallSign;
  myString.toUpperCase();
  myStringLen = myString.length();
  Serial.print("I am sending: "); Serial.println(myString);
}

void setupRFM69() {
  writeReg(0x02,MY_MODULATION);  
  setFrequency(myFrequency);
  writeReg(0x01,0x0C);  // Set mode to Transmitter TX mode
  Serial.println("Transmitter is ready....");
}

struct t_mtab { char c, pat; } ;

struct t_mtab morsetab[] = {
  {'.', 106},
  {',', 115},
  {'?', 76},
  {'/', 41},
  {'A', 6},
  {'B', 17},
  {'C', 21},
  {'D', 9},
  {'E', 2},
  {'F', 20},
  {'G', 11},
  {'H', 16},
  {'I', 4},
  {'J', 30},
  {'K', 13},
  {'L', 18},
  {'M', 7},
  {'N', 5},
  {'O', 15},
  {'P', 22},
  {'Q', 27},
  {'R', 10},
  {'S', 8},
  {'T', 3},
  {'U', 12},
  {'V', 24},
  {'W', 14},
  {'X', 25},
  {'Y', 29},
  {'Z', 19},
  {'1', 62},
  {'2', 60},
  {'3', 56},
  {'4', 48},
  {'5', 32},
  {'6', 33},
  {'7', 35},
  {'8', 39},
  {'9', 47},
  {'0', 63}
} ;

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

void setFrequency(uint32_t freqHz){  // Setup the frequency
  freqHz /= 61; // resolution is 61 Hz so divide it it down and mask it out
  writeReg(0x07, freqHz >> 16);   // Frequency MSB
  writeReg(0x08, freqHz >> 8);    // Frequency Middle Byte
  writeReg(0x09, freqHz);         // Frequency LSB
  Serial.print("Frequency set to "); Serial.println(myFrequency);
}

void txOn(int txTime) {
  writeReg(0x11,MY_TX_POWER); // PA On *******************************************
  delay(txTime);
  writeReg(0x11,0x00); // PA off *************************************************
}

void dash() {
  txOn(DASHLEN);
  delay(DOTLEN);
}

void dit() {
  txOn(DOTLEN);
  delay(DOTLEN);
}

void send(char c) {
  int i ;
  if (c == ' ') {
    //Serial.print(c) ;
    delay(7*DOTLEN) ;
    return ;
  }
  for (i=0; i<N_MORSE; i++) {
    if (morsetab[i].c == c) {
      unsigned char p = morsetab[i].pat ;
      //Serial.print(morsetab[i].c) ;
      while (p != 1) {
          if (p & 1)
            dash() ;
          else
            dit() ;
          p = p / 2 ;
      }
      //tone(cwPin,500,2*DOTLEN);
      delay(2*DOTLEN) ;
      return ;
    }
  }
  /* if we drop off the end, then we send a space */
  //Serial.print("?") ;
}

void sendmsg(char *str) {
  //send('.');  uncomment for fldigi
  while (*str)
    send(*str++) ;
  //Serial.println("");
}


