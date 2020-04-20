// LCD LIBRARY
#include <LiquidCrystal.h>

//DEFINE MACROS FOR LCD PINS
#define LCD_RS 13
#define LCD_E 12
#define LCD_DB4 11
#define LCD_DB5 10
#define LCD_DB6 9
#define LCD_DB7 8

//DC MOTOR/VALVE
#define MOTOR_BIT_FWD 4
#define MOTOR_BIT_REV 5

//INITIALIZE LCD
LiquidCrystal lcd(LCD_RS, LCD_E, LCD_DB4, LCD_DB5, LCD_DB6, LCD_DB7);

//PORT D REGISTERS
volatile unsigned char* portD=(unsigned char*) 0x2B;
volatile unsigned char* ddrD=(unsigned char*) 0x2A;
volatile unsigned char* pinD=(unsigned char*) 0x29;

//MOTOR CONTORL REGISTERS
volatile unsigned char* motorPort=portD;
volatile unsigned char* motorDDR=ddrD;
volatile unsigned char* motorPIN=pinD;

int inches = 0;
int cm = 0;
int distanceThreshold = 5;
int examplePin = 8;
int timeRemaining = 30;

/*
Name: readUltrasonicDistance
Arugments: 
	triggerPin, echoPin
Function:
	The function sends a signal through triggerPin and reports the 
	time it takes to get the signal back over echoPin.
*/
long readUltrasonicDistance(){ // NOTE: ARE ARGUMENTS STILL NEEDED? -KG
  //int triggerPin = 7;
  //int echoPin = 7;
  *ddrD |= B10000000; // pinMode(triggerPin, OUTPUT); Clear the trigger
  *portD &= B01111111; //digitalWrite(triggerPin, LOW);
  delayMicroseconds(2);
  *portD |= B10000000; // digitalWrite(triggerPin, HIGH); Set TriggerPin to high for 10ms
  delayMicroseconds(10);
  *portD &= B01111111; //digitalWrite(triggerPin, LOW);
  *ddrD &= B01111111; //pinMode(echoPin, INPUT);
  return pulseIn(7, HIGH); // Reads the echo pin, and returns the sound wave travel time in microseconds
}

/*
  LCD Circuit specifics:
 * LCD RS to pin 12
 * LCD Enable to pin 11
 * LCD D4 to pin 5
 * LCD D5 to pin 4
 * LCD D6 to pin 3
 * LCD D7 to pin 2
 * LCD R/W to ground
 * LCD VSS to ground
 * LCD VCC to 5V
 * 10K potentiometer:
 	* ends to +5V and ground
 	* wiper to LCD VO
*/


void printLCD(int displayTemp,int displaySec){
// Function: print water temp and seconds left
  lcd.clear();
  printTemp(displayTemp);
  printSec(displaySec);
}
  
void printTemp(int displayTemp){
// Function: print water temp to first row of LCD
 lcd.setCursor(0 ,0);
 lcd.print("Water Temp:");
 lcd.print(displayTemp);
}

float calcTemp(){
// Function: reads from termperature sensor and converts value to fahrenheit
  float tmpReading = analogRead(A0); // read from analog pin A0
  // need following line? or does it read in volts already?
  float volts = (tmpReading / 965.0) * 5;
  float celsius = (volts - .5) * 100;// VOLTS TO CELSIUS
  float fahrenheit = (celsius * 9 / 5 + 32); // CELSUIS TO FAHRENHEIT
  return fahrenheit;
}

void printSec(int displaySec){
// function: print seconds left to second row of LCD
 lcd.setCursor(0,1);
 lcd.print("Seconds Left:");
 lcd.print(displaySec);
}

// VALVE CONTROL
void openValve(){
  *motorPort|=(1<<MOTOR_BIT_FWD); //set forward motor bit to 1
  delay(60); //run motor for a time
  *motorPort&=~(1<<MOTOR_BIT_FWD); //reset forward motor bit to 0
}

void closeValve(){
  *motorPort|=(1<<MOTOR_BIT_REV); //set reverse motor bit to 1
  delay(60); //run motor for a time
  *motorPort&=~(1<<MOTOR_BIT_REV); //reset reverse motor bit to 0
}


/*********************** MAIN CODE **********************/
void setup(){
  Serial.begin(9600);
  pinMode(3, OUTPUT); // GREEN LED SETUP
  pinMode(6, OUTPUT); // RED LED SETUP
  pinMode(A0, INPUT); // TEMP SENSOR SETUP
  lcd.begin(16, 2);// LCD SETUP (COL, ROWS)
  *motorDDR|=(1<<MOTOR_BIT_FWD)|(1<<MOTOR_BIT_REV);
}

void loop(){ 
  // measure the ping time in cm
  cm = 0.01723 * readUltrasonicDistance();
  inches = (cm / 2.54); // cm to inches conversion
  
// IF USER IS NOT WITHIN DISTANCE THRESHOLD
  if (inches > distanceThreshold) {
    *portD &= B10111111; 
    
    // DISPLAY INSERT HANDS MESSAGE
	lcd.clear();
	lcd.setCursor(0,0);
	lcd.print("INSERT HANDS");
    
	digitalWrite(6, HIGH);


	  
// IF USER HAS INSERTED HANDS 
  } else if (inches <= distanceThreshold) {
	*portD |= B01000000; //digitalWrite(examplePin, HIGH); If under threshold, turn LED on
	 lcd.clear();
	 printLCD(calcTemp(), timeRemaining); // DISPLAY TEMP AND TIME REMAINING
     digitalWrite(3, LOW);
     digitalWrite(6, LOW);
	 // OPEN VALVE WHEN FIRST INSERTED
	 if (timeRemaining == 30){
		 openValve();
	 }
	  
	 delay(1000); // DELAY FOR 1 SEC
	 timeRemaining--; // DECREMENT COUNTDOWN BY 1 SEC
	 
	 // IF USER HAS WASHED HANDS FOR 30 SEC
	  if(timeRemaining <= -1){ 
		  lcd.clear();
		  lcd.print("ALL DONE");
          digitalWrite(3, HIGH); // LIGHT GREEN LED
          digitalWrite(6, LOW); // TURN OFF RED LED
		  closeValve(); // STOP HAND WASHING SYSTEM
          delay(2000);
          timeRemaining = 30;
          lcd.clear();
          digitalWrite(3, LOW);
          delay(5000); 
          timeRemaining = 30;
	  }
  }

}
