// CPE 301 AUTOMATED HAND WASHING PROJECT SPR 2020
// 

#include <LiquidCrystal.h>

//DEFINE MACROS FOR LCD PINS
#define LCD_RS 13
#define LCD_E 12
#define LCD_DB4 11
#define LCD_DB5 10
#define LCD_DB6 9
#define LCD_DB7 8

//INITIALIZE LCD
LiquidCrystal lcd(LCD_RS, LCD_E, LCD_DB4, LCD_DB5, LCD_DB6, LCD_DB7);

//DC MOTOR/VALVE
#define MOTOR_BIT_FWD 4
#define MOTOR_BIT_REV 5

//PORT D REGISTERS
volatile unsigned char* portD=(unsigned char*) 0x2B;
volatile unsigned char* ddrD=(unsigned char*) 0x2A;
volatile unsigned char* pinD=(unsigned char*) 0x29;

//MOTOR CONTORL REGISTERS
volatile unsigned char* motorPort=portD;
volatile unsigned char* motorDDR=ddrD;
volatile unsigned char* motorPIN=pinD;

//GLOBAL VARIABLES
int inches = 0;
int cm = 0;
int distanceThreshold = 5;
int temperatureThreshold = 100; // 100 F
int examplePin = 8;
int timeRemaining = 30;
bool pushButtonPressed = false;

long readUltrasonicDistance(){
// Function: sends a signal through triggerPin and reports the time it takes to get the signal back over echoPin
  *ddrD |= B10000000; // pinMode(triggerPin, OUTPUT); Clear the trigger
  *portD &= B01111111; //digitalWrite(triggerPin, LOW);
  delayMicroseconds(2);
  *portD |= B10000000; // digitalWrite(triggerPin, HIGH); Set TriggerPin to high for 10ms
  delayMicroseconds(10);
  *portD &= B01111111; //digitalWrite(triggerPin, LOW);
  *ddrD &= B01111111; //pinMode(echoPin, INPUT);
  return pulseIn(7, HIGH); // Reads the echo pin, and returns the sound wave travel time in microseconds
}

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
  *portD|=(1<<2); //enable pullup for pushbutton
}

void loop(){ 
	// MEASURE DISTANCE SENSOR IN INCHES
	cm = 0.01723 * readUltrasonicDistance();
	inches = (cm / 2.54);

/*	if(*pinD&(1<<2))==(1<<2)){
		pushButtonPressed = true;
	}
*/	
	if(pushButtonPressed = false){
		// system does not start
	}
	else if (pushButtonPressed = true){
		openValve();
		while(inches > distanceThreshold){
			cm = 0.01723 * readUltrasonicDistance();
			inches = (cm / 2.54);
			*portD |= B01000000; // RED LIGHT ON
			timeRemaining = 30; // RESET TIMER
          	lcd.clear();
			printTemp(calcTemp());
			if(calcTemp() < temperatureThreshold){
				lcd.setCursor(0,1);
				lcd.print("INCREASE TEMP");
				delay(2000); // delay so user can read message
			}
			lcd.clear();
			printTemp(calcTemp());
			lcd.setCursor(0,1);
			lcd.print("INSERT HANDS");
			*portD &= B10111111; // RED LIGHT OFF
		}
		
		// USER IS WASHING HANDS
		do{
			//clear lcd?
			cm = 0.01723 * readUltrasonicDistance();
			inches = (cm / 2.54);
			printLCD(calcTemp(), timeRemaining);
			delay(1000);
			timeRemaining--;
		}while((inches <= distanceThreshold) && (timeRemaining >= 0));
	}
	
	// WHEN USER HAS WASHED HANDS FOR A SUFFICIENT AMOUNT OF TIME
	if(timeRemaining <= 0){
		*portD |= B00001000; // GREEN LED ON
		lcd.clear();
		lcd.print("ALL DONE");
      	delay(1000);
		lcd.clear();
		*portD &= B11110111; // GREEN LED OFF
      	delay(5000); 
	}
	timeRemaining = 30; 
}
		


/* Notes:
+ DIGITAL WRITE USING REGISTERS: 
    *portD |= B01000000 EQUIV TO digitalWrite(6, HIGH)
    *portD &= B10111111 EQUIV TO digitalWrite(6, LOW)
    
*/
