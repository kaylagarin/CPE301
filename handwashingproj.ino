// CPE 301 AUTOMATED HAND WASHING PROJECT SPR 2020
// 

#include <LiquidCrystal.h>

#define WASH_TIME 30

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
int timeRemaining = WASH_TIME;
bool normalOperation = true;

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
 lcd.print("F");
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

//LED Blink
void LED_Blink(int seconds_blink){
  for( ; seconds_blink>0; seconds_blink--){
  	*portD |= B01000000; // RED LIGHT ON
	delay(500);
  	*portD &= B10111111; // RED LIGHT OFF
  	delay(500);
  }
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

	if(normalOperation == true){
		// wait for pushbutton, then turn on valve
      while((*pinD&(1<<2))==(1<<2));
      openValve();
	}

    while(inches > distanceThreshold){
        timeRemaining = WASH_TIME; //RESET TIMER
        lcd.clear();
        printTemp(calcTemp());
        if(calcTemp() < temperatureThreshold){
            lcd.setCursor(0,1);
            lcd.print("INCREASE TEMP");
            delay(500); // delay so user can read message
        }
        lcd.clear();
        printTemp(calcTemp());
      if(calcTemp() >= temperatureThreshold){
          	lcd.setCursor(0,1);
          	lcd.print("INSERT HANDS");
        	delay(500);
      }
      	cm = 0.01723 * readUltrasonicDistance();
		inches = (cm / 2.54);
    }
    
    // USER IS WASHING HANDS
    do{
        printLCD(calcTemp(), timeRemaining);
        delay(1000);
        timeRemaining--;
        cm = 0.01723 * readUltrasonicDistance();
        inches = (cm / 2.54);      
    }while((inches <= distanceThreshold) && (timeRemaining > 0));
  	
    if(inches>distanceThreshold){
      normalOperation=false;
      lcd.clear();
      LED_Blink(2);
    }
	
	// WHEN USER HAS WASHED HANDS FOR A SUFFICIENT AMOUNT OF TIME
	else if(timeRemaining <= 0){
        closeValve();
		*portD |= B00001000; // GREEN LED ON
		lcd.clear();
      	lcd.setCursor(0,0);
		lcd.print("ALL DONE");
      	delay(5000);
		lcd.clear();
		*portD &= B11110111; // GREEN LED OFF
        normalOperation=true;
	}
    else{
        normalOperation=false;
      	lcd.clear();
        LED_Blink(2);//blink red light
        
    }

	timeRemaining = WASH_TIME; 
}
