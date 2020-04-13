// SPRING 2020 CPE 301 LAB 7
// KAYLA GARIN

// DESCRIPTION: CODE TO INCREMENT A 7 SEGMENT LED FROM 0-F USING A PUSH BUTTON
// COMPONENTS: ARDUINO UNO, 7 SEG LED, PUSH BUTTON

// NOTE: UNSURE WHY PIN G IS ALWAYS LIT - MAY BE A TINKERCAD ERROR // 

/* ----------------------7 SEG KEY----------------------------------------
0b(pins in decreasing order starting from 7)
0 = LOW OR LED OFF ... 1 = HIGH OR LED ON
0b11111100  ==  '0' -- 0b11101110  == 'A'
0b01100000  ==  '1' -- 0b00111110  == 'b'
0b11011010  ==  '2' -- 0b10011100  == 'C'
0b11110010  ==  '3' -- 0b01111010  == 'd'
0b01100110  ==  '4' -- 0b10011110  == 'E'
0b10110110  ==  '5' -- 0b10001110  == 'F'
0b10111110  ==  '6' 
0b11100000  ==  '7' 
0b11111110  ==  '8' 
0b11110110  ==  '9' 
-------------------------------------------------------------------- */

volatile unsigned char * port_d = (unsigned char *) 0x2B;
volatile unsigned char * ddr_d = (unsigned char *) 0x2A;
volatile unsigned char * pin_d = (unsigned char *) 0x29;

unsigned int counter = 0;
int state;

// ARRAY OF VALUES IN ORDER FROM 0-F
char displayValue[16] = {0b11111100, 0b01100000, 0b11011010, 0b11110010,
                         0b01100110, 0b10110110, 0b10111110, 0b11100000,
                         0b11111110, 0b11110110, 0b11101110, 0b00111110,
                         0b10011100, 0b01111010, 0b10011110, 0b10001110};

void setup(){
// 7 SEG LED SET UP
  *ddr_d |= 0b11111111;
  *port_d &= 0b00000000;
// PUSH BUTTON SET UP
  pinMode(8,INPUT_PULLUP);
  Serial.begin(9600);
}

void loop(){
  state = digitalRead(8);
  *port_d = displayValue[counter];
  if (state == LOW){
    /*DEBOUNCING (NOT NEEDED ON TINKER CAD BUT GOOD FOR REAL LIFE) 
    for(int i = 0; i < 1000; i++); 
    if (state == HIGH) {
      break;
    } 
    */ 
    while(digitalRead(8) == LOW);
    if(counter == 15){ // IF DISPLAYING F
      counter = -1; // SET TO -1 SO COUNTER IS 0 ON NEXT PUSH
    }
    counter+=1; // INCREMENT COUNTER 
    
    Serial.println(counter);
    
  }
}