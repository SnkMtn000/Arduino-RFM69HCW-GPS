/*
 * My atempt to create a simple Morse code sender
 *  KC1ENN
 * for rfm69hcw
 * on a ProMini 3.3
 * 
 * 1) force all characters to upper case - failed
 * 2) add command for CQ as /CQ but first need to capture and print string
 * 
 * with thanks to Mark VandeWettering
 * 
 * "This code is so trivial that I'm releasing it completely without restrictions."
 * Simple Arduino Morse Beacon v0.0
 * Written by Mark VandeWettering <k6hx@arrl.net>
 * Check out my blog @ http://brainwagon.org
 */
 
//  RTTY works!  ProMini 3.3
// Standard RTTY 5N1.5 170 45.45
// RFM69HCW

#include <SPI.h>

#define MODULATION 0x60
#define POWER 0x58  // 6 dBm
//#define POWER 0x5B  //  9 dBm
//#define POWER 0x5E  // 12 dBm
//#define POWER 0x7F  // 17 dBm
#define ssPin 10  //  SS
#define rstPin 9 //  reset pin
#define myDelay 5
#define myShift 3 // 3 x 61 Hz = 181 Hz ~ 170 Hz


String myString = "";
int myStringLen = 0;
int current_state;
int previous_state;
int send_shift;
float baud = 45.45;
float bit_time = 1000/baud;;
float stop_bit_time = bit_time * 2;
int index = 0;
String inputString = "";         // a String to hold incoming data

String myCall = "... CQ  CQ  CQ DE  MYCALLSIGN  MYCALLSIGN  MYCALLSIGN PSE K ...    ";
boolean stringComplete = false;  // whether the string is complete
uint32_t myMark;
uint32_t mySpace;
long myFrequency = 433200000;  //  70cm
//long myFrequency = 915000000;  //  33cm        

void setup(){
  Serial.begin(9600);
  Serial.println("Starting up");
  // reserve 200 bytes for the inputString:
  inputString.reserve(200);
  pinMode(ssPin, OUTPUT);
  pinMode(rstPin, OUTPUT);
  setupSPI();
  delay(1000);
  resetRFM69();
  delay(1000);
  setupRFM69();
}

void loop (){
  if (stringComplete) {
  // clear the string:
  inputString = "";
  //transmitData();
  stringComplete = false;
  }
} //ends void loop
///////////////////////////////////////////////

void serialEvent() {
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read();
    // add it to the inputString:
    inputString += inChar;
    // if the incoming character is a newline, set a flag so the main loop can
    // do something about it:
    if (inChar == '\n') {
      stringComplete = true;
      if(inputString.indexOf("@freq=") == 0) {
        myFrequency = inputString.substring(6,15).toInt();
        setFrequency(myFrequency);
        getMark(myFrequency);
        getSpace(myFrequency);
        Serial.print("Frequency now set to: "); Serial.println(inputString.substring(6,15));
        inputString = "";
        myString = "";
        myStringLen = 0;
      }
      if(inputString.indexOf("@freq+") == 0) {
        myFrequency = myFrequency + 61;
        setFrequency(myFrequency);
        getMark(myFrequency);
        getSpace(myFrequency);
        Serial.print("Frequency now set to: "); Serial.println(myFrequency);
        inputString = "";
        myString = "";
        myStringLen = 0;
      } 
      if(inputString.indexOf("@freq-") == 0) {
        myFrequency = myFrequency - 61;
        setFrequency(myFrequency);
        getMark(myFrequency);
        getSpace(myFrequency);
        Serial.print("Frequency now set to: "); Serial.println(myFrequency);
        inputString = "";
        myString = "";
        myStringLen = 0;
      }
      if (inputString.indexOf("@cq") == 0) {
        myString = myCall;
        myString.toUpperCase();
        myStringLen = myString.length();
        Serial.print("I am sending: "); Serial.println(myString);
        transmitData();
      } 
      else {
        inputString.trim();
        if(inputString.length() > 0) {
          myString = "   " +inputString + "    ";
          myString.toUpperCase();
          myStringLen = myString.length();
          Serial.print("I am sending: "); Serial.println(myString);
          transmitData();
        }
      }
    }
  }
}

void resetRFM69() {
  digitalWrite(rstPin, HIGH);
  delay(100);
  digitalWrite(rstPin, LOW);
  delay(100);
}

void setupRFM69() {
  writeReg(0x01,0x84); // set RegOpMode to Sequencer On Listen Off Standby = 0x04
  writeReg(0x02,MODULATION); // RegDataModul = Continuous mode w/0 synthesizer FSK No Shaping = 0x60
  setFrequency(myFrequency);
  Serial.print("Frequency set to "); Serial.println(myFrequency);
  getMark(myFrequency);
  getSpace(myFrequency);
  writeReg(0x26,0x07);  //  CLK off to save power
  writeReg(0x11,0x70);  // 0x9F PA0 On only, 0x5F or 0x41  //  Pa1On 0x40 to 0x5F ?????
  Serial.println("Transmitter is ready....");
}

void printReg(byte data){
  Serial.print("Register "); 
  Serial.print(data); 
  Serial.print(" = "); 
  Serial.println(readReg(data), HEX);
}

void writeReg(uint8_t addr, uint8_t value)
{
  digitalWrite(ssPin,LOW);
  SPI.transfer(addr | 0x80);
  SPI.transfer(value);
  digitalWrite(ssPin, HIGH);
}

uint8_t readReg(uint8_t addr)
{
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
}

void getMark(uint32_t freqHz) {
  freqHz /= 61; // divide down by FSTEP to get FRF
  myMark = freqHz;
}

void getSpace(uint32_t freqHz) {
  freqHz /= 61; // divide down by FSTEP to get FRF
  mySpace = freqHz - 3;
}

///////////////////////////////////////////////

void transmitData(){
  writeReg(0x01,0x0C);
  delay(5000);
  for (index = 0; index < myStringLen; index++)  {
        check_current_state(myString[index]) ;
        compare_state();
        send_shift_sig();
        lookup_send(myString, index);
    }
    writeReg(0x01,0x84); // set RegOpMode to Sequencer On Listen Off Standby = 0x04
    delay(1000);
}

void check_current_state(char val) {
  if (isalpha(val)) 
  current_state = 0;
  else 
  current_state = 1;
  } 
    
void compare_state() {  //compares states and sends letter shifts
    if (current_state == previous_state) { 
        send_shift = 0;
        }
    else { 
        send_shift = 1;
        previous_state = current_state;
        }
       }
       
void send_shift_sig() {
    if (send_shift == 1) {
        if (current_state == 0) {
            letter_shift(); }
        else { figure_shift(); }
        }
        if (current_state == 1) {  // My addition
      figure_shift();        // my addition 
        }
} 
    
void lookup_send(String checkletter, int indexofstring) {       
    if  (checkletter[indexofstring] == 'A' || checkletter[indexofstring] == '-'){//11000
        start_bit();
        markF();
        markF();
        spaceF();
        spaceF();
        spaceF();
        stop_bit();}
    else if (checkletter[indexofstring] == 'B' || checkletter[indexofstring] == '?') {   //10011
        start_bit();
        markF();
        spaceF();
        spaceF();
        markF();
        markF();
        stop_bit(); }
    else if (checkletter[indexofstring] == 'C' || checkletter[indexofstring] == ':') {   //01110
        start_bit();
        spaceF();
        markF();
        markF();
        markF();
        spaceF();
        stop_bit(); }
    else if (checkletter[indexofstring] == 'D') {   //10010 WRU? will program later
        start_bit();
        markF();
        spaceF();
        spaceF();
        markF();
        spaceF();
        stop_bit(); }
    else if (checkletter[indexofstring] == 'E' || checkletter[indexofstring] == '3') {   //10000
        start_bit();
        markF();
        spaceF();
        spaceF();
        spaceF();
        spaceF();
        stop_bit(); }
    else if (checkletter[indexofstring] == 'F' || checkletter[indexofstring] == '!') {   //10110
        start_bit();
        markF();
        spaceF();
        markF();
        markF();
        spaceF();
        stop_bit(); }
    else if (checkletter[indexofstring] == 'G' || checkletter[indexofstring] == '&') {   //01011
        start_bit();
        spaceF();
        markF();
        spaceF();
        markF();
        markF();
        stop_bit(); }
    else if (checkletter[indexofstring] == 'H' || checkletter[indexofstring] == '#') {   //00101
        start_bit();
        spaceF();
        spaceF();
        markF();
        spaceF();
        markF();
        stop_bit(); }
    else if (checkletter[indexofstring] == 'I' || checkletter[indexofstring] == '8') {   //01100
        start_bit();
        spaceF();
        markF();
        markF();
        spaceF();
        spaceF();
        stop_bit(); }
    else if (checkletter[indexofstring] == 'J') {   //11010 should send a ' as well, but errors
        start_bit();
        markF();
        markF();
        spaceF();
        markF();
        spaceF();
        stop_bit(); }
    else if (checkletter[indexofstring] == 'K' || checkletter[indexofstring] == '(') {   //11110
        start_bit();
        markF();
        markF();
        markF();
        markF();
        spaceF();
        stop_bit(); }
    else if (checkletter[indexofstring] == 'L' || checkletter[indexofstring] == ')') {   //01001
        start_bit();
        spaceF();
        markF();
        spaceF();
        spaceF();
        markF();
        stop_bit(); }
    else if (checkletter[indexofstring] == 'M' || checkletter[indexofstring] == '.') {   //00111
        start_bit();
        spaceF();
        spaceF();
        markF();
        markF();
        markF();
        stop_bit(); }
    else if (checkletter[indexofstring] == 'N' || checkletter[indexofstring] == ',') {   //00110
        start_bit();
        spaceF();
        spaceF();
        markF();
        markF();
        spaceF();
        stop_bit(); }
    else if (checkletter[indexofstring] == 'O' || checkletter[indexofstring] == '9') {   //00011
        start_bit();
        spaceF();
        spaceF();
        spaceF();
        markF();
        markF();
        stop_bit(); }
    else if (checkletter[indexofstring] == 'P' || checkletter[indexofstring] == '0') {   //01101
        start_bit();
        spaceF();
        markF();
        markF();
        spaceF();
        markF();
        stop_bit(); }
    else if (checkletter[indexofstring] == 'Q' || checkletter[indexofstring] == '1') {   //11101
        start_bit();
        markF();
        markF();
        markF();
        spaceF();
        markF();
        stop_bit(); }
    else if (checkletter[indexofstring] == 'R' || checkletter[indexofstring] == '4') {   //01010
        start_bit();
        spaceF();
        markF();
        spaceF();
        markF();
        spaceF();
        stop_bit(); }
    else if (checkletter[indexofstring] == 'S') {   //10100 should do Bell?
        start_bit();
        markF();
        spaceF();
        markF();
        spaceF();
        spaceF();
        stop_bit(); }
    else if (checkletter[indexofstring] == 'T' || checkletter[indexofstring] == '5') {   //00001
        start_bit();
        spaceF();
        spaceF();
        spaceF();
        spaceF();
        markF();
        stop_bit(); }
    else if (checkletter[indexofstring] == 'U' || checkletter[indexofstring] == '7') {   //11100
        start_bit();
        markF();
        markF();
        markF();
        spaceF();
        spaceF();
        stop_bit(); }
    else if (checkletter[indexofstring] == 'V' || checkletter[indexofstring] == ';') {   //01111
        start_bit();
        spaceF();
        markF();
        markF();
        markF();
        markF();
        stop_bit(); }
    else if (checkletter[indexofstring] == 'W' || checkletter[indexofstring] == '2') {   //11001
        start_bit();
        markF();
        markF();
        spaceF();
        spaceF();
        markF();
        stop_bit(); }
    else if (checkletter[indexofstring] == 'X' || checkletter[indexofstring] == '/') {   //10111
        start_bit();
        markF();
        spaceF();
        markF();
        markF();
        markF();
        stop_bit(); }
    else if (checkletter[indexofstring] == 'Y' || checkletter[indexofstring] == '6') {   //10101
        start_bit();
        markF();
        spaceF();
        markF();
        spaceF();
        markF();
        stop_bit(); }
    else if (checkletter[indexofstring] == 'Z' || checkletter[indexofstring] == '"') {   //10001
        start_bit();
        markF();
        spaceF();
        spaceF();
        spaceF();
        markF();
        stop_bit(); }
    else if ( (checkletter[indexofstring] == ' ')) {//this seems to properly send a space 00100. for a new line i think i need 01000
        start_bit();
        spaceF();
        spaceF();
        markF();
        spaceF();
        spaceF();
        stop_bit(); }
   
    
    else {                      //if no valid characters are provided a ? is returned for that character
        figure_shift();
        start_bit();
        markF();
        spaceF();
        spaceF();
        markF();
        markF();
        stop_bit();  }
    }
        
void start_bit() {
    writeReg(0x09,mySpace); //  my addition
    //tone(pin,space);
    delay(bit_time);
                  }

   
void markF() {
    writeReg(0x09,myMark); //  my addition        
    //tone(pin,mark);
    delay(bit_time);
    }
    
void spaceF() {
  writeReg(0x09,mySpace);  //  my addition
    //tone(pin,space);
    delay(bit_time);
    }
    
void stop_bit() {
    writeReg(0x09,myMark); //  my addition
    //tone(pin,mark);
    delay(stop_bit_time);
    }
    
void figure_shift() {
    //switch to figures   
    start_bit();
    //11011
    markF();
    markF();
    spaceF();
    markF();
    markF();
    //stop bit
    stop_bit();
    }
void letter_shift() {
    //switch to letters   
    start_bit();
    //11011
    markF();
    markF();
    markF();
    markF();
    markF();
    //stop bit
    stop_bit();
    }

//
//Written by Ted Van Slyck www.openrcdesign.com
//Feel free to use for personal non commercial use
//use at your own risk, no warranties. It probably has errors of some kind. 

//this program has an issue with finishing a string on a non space. perhaps i should program it to send a new line or a space at the end of all strings regardless? this MIGHT be an issue with how droidRTTY interperets too...
//Also, i think i should make it send a letter or a figure shift each time it commences a string.
               
