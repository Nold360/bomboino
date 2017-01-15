#include <LiquidCrystal.h>
#include <EEPROM.h>

//IO Pin definition
#define buttonPin 8     
#define lockPin   10
#define ledPin    13     
#define piezoPin  9

// State Definitions
#define ARMED 0
#define DISARMED 1

// Time it takes to arm/disarm the bomb in ms
int armDelay = 5000;

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

// Don't change this values
int buttonState = LOW; 
int lockState = LOW; 
int detonationDelay = 5;
int timeLeft = 0;
boolean isArmed = false;
boolean wroteMem = false;
String text, ani, prefix;
int minutes=0, seconds=0;
int buttonPressed = 0;
int memAddr = 0;

void setup() {
  int memVal = eepromReadInt(memAddr);

  if(memVal <= 60 && memVal >= 5) {
    detonationDelay = memVal;
  }
  
  pinMode(ledPin, OUTPUT); 
  pinMode (piezoPin, OUTPUT);

  pinMode(buttonPin, INPUT); 
  pinMode(lockPin, INPUT);   

  // set up the LCD's number of columns and rows: 
  lcd.begin(40, 2);

  // Print a message to the LCD.  
  lcdPrintln("S.O.F - Airsoft");
  playSound(ARMED);
  delay(1000);
  lcdPrintln("Bombuino Ver.1.0");
  delay(1000);
}

void loop() {
  buttonState = digitalRead(buttonPin);
  lockState = digitalRead(lockPin);

  if (!isArmed) { 
    /******************************* Set-Time/Orga-Mode ****************************************/
    if(lockState == LOW) {
      wroteMem = false;
      text = "Set Time: ";
      text += detonationDelay;
      text += " min";
      lcdPrintln(text); 
      delay(200);
    } 
    else {
      if(!wroteMem) {
        eepromWriteInt(memAddr, detonationDelay);
        wroteMem = true;  
      } 
      lcdPrintln(".... READY ....."); 
      delay(500);     
    }

    /******************************* ARMing ****************************************/
    if(buttonState == HIGH && lockState == HIGH) {
      buttonPressed = 0;
      ani = "+";
      //ARM if button is pressed for armDelay    
      while(digitalRead(buttonPin) == HIGH && !isArmed) {
        lcdPrintln("ARMING: " + ani);
        ani = ani + "+";
        delay(1000);  
        buttonPressed += 1000;
        if (buttonPressed >= armDelay) {
          lcdPrintln(" !!! ARMED !!!");
          timeLeft = detonationDelay*60; //min to sec
          isArmed = true;
          playSound(ARMED);
          delay(2000);
        }
      }
    } 

    //If NOT locked, change Time if button is pressed
    else if(buttonState == HIGH && lockState == LOW) {
      if(detonationDelay > 60) {
        detonationDelay = 5;
      } 
      else {
        detonationDelay += 5; 
      }
    } 

    /******************************* ARMED-Mode ****************************************/
  } 
  else { //if bomb is armed
    //Count down if button isn't pressed
    if(buttonState == LOW) {
      timeLeft -= 1;
      text = "Time left: ";
      prefix = "";

      /******************************* EXPLOSION ****************************************/
      if(timeLeft <= 0) {
        tone(piezoPin, 2500, 5000);
        lcdPrintln("!Terrorists Win!");

        //Wait for Orga to unlock 
        while(digitalRead(lockPin) == HIGH); 
        isArmed = false;

        /******************************* SHOW COUNTDOWN ****************************************/
      } 
      else {
        minutes=0;
        seconds=0;
        minutes = timeLeft / 60;
        seconds  = timeLeft % 60;

        if(seconds < 10)
          prefix = "0";
        else
          prefix = "";

        text += minutes;
        text += ":";
        text += prefix;
        text += seconds;
        lcdPrintln(text);
        tone(piezoPin, 2500, 150);

        //beep faster in last 30 seconds
        if(timeLeft < 30) {
          delay(200);
          tone(piezoPin, 2500, 150); 
          delay(800);
        } 
        else {
          delay(1000);
        }
      }
    } 

    /******************************* DEFUSE ****************************************/
    else if(buttonState == HIGH) {
      buttonPressed = 0; 
      text = "+";
      //DISARM if button is pressed for armDelay    
      while(digitalRead(buttonPin) == HIGH && isArmed) {
        lcdPrintln("DEFUSING: " + text);
        text += "+";
        delay(1000);  
        buttonPressed += 1000;
        if (buttonPressed >= armDelay) {
          lcdPrintln("+ BOMB DEFUSED +");
          isArmed = false;
          playSound(DISARMED);
          while(digitalRead(lockPin) == HIGH); 
        }
      }
    }
  }
}

/*
Function: lcdPrintln
 Params: String string
 Return: void
 Desc: My LCD Sucks... It's 1x16.. but logically 2x40
 So this is my workaround
 */
void lcdPrintln(String string) {
  string = string +"                ";
  lcd.clear();
  lcd.setCursor(0, 0);
  for(int i=0; i<8; i++) {
    lcd.print(string[i]);  
  }

  lcd.setCursor(0, 1);
  for(int i=8; i<16; i++) {
    lcd.print(string[i]);  
  }
}

/*
Function: playSound
 Params: int Sound - Sound to play
 Return: void
 Desc: Playes a Melodie, choosen by ARMED or DISARMED
 */
void playSound(int sound) {
  if(sound == ARMED) {
    tone(piezoPin, 1000, 500);
    delay(200);
    tone(piezoPin, 1500, 500);
    delay(200);
    tone(piezoPin, 2000, 200);
    delay(200);
  } 
  else if (sound == DISARMED) {
    tone(piezoPin, 2000, 500);
    delay(200);
    tone(piezoPin, 1500, 500);
    delay(200);
    tone(piezoPin, 1000, 200);
    delay(200); 
  }
}

//int = 2byte, eeprom only 1 byte.. 
//thanks to http://shelvin.de/eine-integer-zahl-in-das-arduiono-eeprom-schreiben/
void eepromWriteInt(int adr, int wert) {
  byte low, high;
  low=(wert&0xFF);
  high=((wert>>8)&0xFF);
  EEPROM.write(adr, low); // dauert 3,3ms 
  EEPROM.write(adr+1, high);
  return;
} //eepromWriteInt

int eepromReadInt(int adr) {
  byte low, high;
  low=EEPROM.read(adr);
  high=EEPROM.read(adr+1);
  return low + ((high << 8)&0xFF00);
} //eepromReadInt

