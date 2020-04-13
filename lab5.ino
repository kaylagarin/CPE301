// SPRING 2020 CPE 301 LAB 5

volatile unsigned char *myTCCR1A = (unsigned char*) 0x80;
volatile unsigned char *myTCCR1B = (unsigned char*) 0x81;
volatile unsigned char *myTCCR1C = (unsigned char*) 0x82;
volatile unsigned char *myTIMSK1 = (unsigned char*) 0x6F;
volatile unsigned int  *myTCNT1  = (unsigned int*) 0x84;
volatile unsigned char *myTIFR1 =  (unsigned char*) 0x36;
volatile unsigned char *portDDRB = (unsigned char*) 0x24;
volatile unsigned char *portB = (unsigned char*) 0x25;
volatile unsigned char *pinB = (unsigned char*) 0x23;
volatile unsigned char *ddrB = (unsigned char*) 0x24;
unsigned char in_char;
char arr_1[] = {'A', 'a', 'B', 'C', 'c', 'D', 'd', 'E', 'F', 'f', 'G', 'g', 'q'};
int arr_2[] = {440, 466, 494, 523, 554, 587, 624, 659, 698, 740, 784, 831, 0};

void setup() 
{
  //Set up all the required registers
  *myTCCR1A = 0x00; //set to 0
  *myTCCR1B = 0x00; //set prescalar to 1
  *myTCCR1C = 0x00; //set to 0
  *ddrB |= 0x40; //set pin 12 to OUTPUT
  *portB &= 0xBF; // port 6 set to low
  Serial.begin(9600);
}
void loop() 
{
  int ticks;
  //Read input from PC Serial Port
  if(Serial.available())
  {
    in_char = Serial.read();
    Serial.write(in_char);
    for(int i=0; i<13; i++){
      if(in_char == arr_1[i]){
        if(arr_2[i] != 0){
          ticks = get_ticks(arr_2[i]); //Get number of ticks based on Frequency
        } 
        else{
          ticks = 0;
        }
      }
    }
    Serial.println(ticks);
  } 
  if(ticks == 0){
    *portB &= 0xBF;
  }
  else{
    *portB |= 0x40; //Set port B6 to high,
    my_delay(ticks); //Custom delay
    *portB &= 0xBF; //Set port B6 to low.
    my_delay(ticks); //Custom delay
  }
}

unsigned int get_ticks(unsigned int freq){
  /*
  Calculate ticks based on frequency and return
  */

  float period = (1.0 / freq); // calc period
  period = (period / 2.0); // 50% duty cycle
  float clk = (1.0 / 16000000.0); // clock period def
  int ticks1 = (period / clk); // calc ticks
  return ticks1;
}

void my_delay(unsigned int ticks)
{
  *myTCCR1B &= 0xF8; // stop the timer
  *myTCNT1 = (unsigned int) (65536 - ticks); // set the counts
  *myTCCR1B |= 0b00000001; // start the timer
  while((*myTIFR1 & 0x01)==0); // wait for overflow
  *myTCCR1B &= 0xF8; // stop the timer
  *myTIFR1 |= 0x01; // reset TOV
}

