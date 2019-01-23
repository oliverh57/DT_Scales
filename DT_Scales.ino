#include "HX711.h" //include lib for scale ADC
HX711 scale(7,8); //initiate scale on pins 7 & 

#include <LCD.h> //include LCD libs 
#include <LiquidCrystal_I2C.h> //i2c LIB for LCD 

#define I2C_ADDR 0x27  // Define I2C Address where the PCF8574A is
#define BACKLIGHT_PIN 3
#define En_pin 2
#define Rw_pin 1
#define Rs_pin 0
#define D4_pin 4
#define D5_pin 5
#define D6_pin 6
#define D7_pin 7

LiquidCrystal_I2C lcd(I2C_ADDR, En_pin, Rw_pin, Rs_pin, D4_pin, D5_pin, D6_pin, D7_pin); //Decalre LCD

void lcdREFRESH(){
  lcd.clear(); //refresh lcd
  lcd.setCursor(1, 0);// set lcd to "home"
  
}


//TEAR
#define tear_pin 10 //pin for tear button
int tear_offset = 0; //declare the tear veriable as a global

int read_scale_adjusted(bool tear){ //return the adjusteed readering of the scale
  //int RAW_read = scale.read(); //read the scale for procesing later
  //int MAP_read = (RAW_read+198729);
  //MAP_read = MAP_read/-369;

  int MAP_read = map(scale.read(), -196601.7354, -208234.47, 0, 41);
  delay(100);
  
  if(tear == true){
    return(MAP_read+tear_offset);
  }else{
   return(MAP_read); 
  }
}


//FOODS
String foods[3] = {"Flower  ", "Eggs    ", "Suggar3   "};


//SHIFT REGISTER 
#define SER    6  // data in
#define SRCLK  5  // shift register clock
#define SRCLR  4  // clear shift register
#define RCLK   3  // storage register

// clear the shift registers without affecting the storage registers.
void clearShiftRegisters() {
  digitalWrite(SRCLR,LOW);
  digitalWrite(SRCLR,HIGH);
}

// All the data in the shift registers moves over 1 unit and the new data goes in at shift register 0.
// The data that was in the shift register 7 goes to the next register (if any).
void shiftDataIn(int data) {
  digitalWrite(SER,data);
  digitalWrite(SRCLK,HIGH);
  digitalWrite(SRCLK,LOW);
}

// copy the 8 shift registers into the shift registers,
// which changes the output voltage of the pins.
void copyShiftToStorage() {
  digitalWrite(RCLK,HIGH);
  digitalWrite(RCLK,LOW);
}

//output two numbers to the shift registers.
void outputLED(int v1, int v2){
  clearShiftRegisters();
  for (int i = 0; i < 10; i++){
    if(v1>0){
      shiftDataIn(1);
      v1=v1-1;
    }else{
      shiftDataIn(0);
    }

  }

    for (int i = 0; i < 10; i++){
    if(v2>0){
      shiftDataIn(1);
      v2=v2-1;
    }else{
      shiftDataIn(0);
    }
  }
  copyShiftToStorage();

}//end outputLED


//ROTARY ENCODER
#define DT 2 //pin for rot encoder
#define CLK 9 //pin for rot encoder
unsigned short int encoder0POS = 0; //value for rot encoder mode A

volatile bool turndetected;
volatile bool up;

void encoderisr ()  {
  turndetected = true;
  up = (digitalRead(CLK) == digitalRead(DT));
}

void encoderREFRESH(){ //function to update encoder
  if (turndetected) {//if isr flag is set
    if(up){
      encoder0POS++;
    }else{
      encoder0POS--;
    }
    turndetected = false; //rest isr flag
    
    #if defined(SERIAL_DEBUG)
      Serial.println("ENCODER REFRESHED");
    #endif
  }
}//end encoderREFRESH 

void setup() {
  pinMode(13, OUTPUT);
  
  lcd.begin(16, 2); //begin LCD
  lcd.setBacklightPin(BACKLIGHT_PIN, POSITIVE);// Switch on the backlight
  lcd.setBacklight(HIGH);
  lcd.home();  // go home
  
  pinMode(SER,OUTPUT);
  pinMode(SRCLK,OUTPUT);
  pinMode(SRCLR,OUTPUT);
  pinMode(RCLK,OUTPUT);

  pinMode(tear_pin, INPUT_PULLUP);
  
  clearShiftRegisters();

  lcdREFRESH();//clear LCD 

  tear_offset = 0 - read_scale_adjusted(false);
  Serial.begin(9600);

  attachInterrupt (digitalPinToInterrupt(2),encoderisr,FALLING); //rotary encoder isr
}

void loop() {
  encoderREFRESH();//refresh encoder
   
   //outputLED(map(read_scale_adjusted(true), 0, 50,0,10), 5);
  outputLED(10,10);
  char msg[21];
  
  lcd.setCursor(0, 0);// Next line
  lcd.print("Food: " + foods[encoder0POS % 3]);

 
  
  sprintf(msg, "Mass: %-7d", encoder0POS);
  lcd.setCursor(0, 1);// Next line
  lcd.print(msg);




   if(digitalRead(tear_pin) == LOW){//if the tear button has been pressed
    tear_offset = 0 - read_scale_adjusted(false);
    digitalWrite(13,HIGH);
   }
}
