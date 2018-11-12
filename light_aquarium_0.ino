// ambiental aqauarium light - 11.11.2018 
// based on Dawn & Dusk controller. http://andydoz.blogspot.ro/2014_08_01_archive.html
// 16th August 2014. (C) A.G.Doswell 2014
// adapted sketch by niq_ro from http://nicuflorica.blogspot.ro & http://arduinotehniq.blogspot.com/
// Date and time functions using a DS1307 RTC connected via I2C and Wire lib

// Time is set using a rotary encoder with integral push button. The Encoder is connected to interrupt pins D2 & D3 (and GND), 
// and the push button to pin A0 (and GND)
// The RTC is connections are: Analogue pin 4 to SDA. Connect analogue pin 5 to SCL.
// A 2 x 16 LCD display is connected as follows (NOTE. This is NOT conventional, as interrupt pins are required for the encoder)
//  Arduino LCD  
//  D4      DB7
//  D5      DB6
//  D6      DB5
//  D7      DB4
//  D12     RS
//  D13     E
// 
// Use: Pressing and holding the button will enter the clock set mode (on release of the button). Clock is set using the rotary encoder. 
// The clock must be set to UTC.
// Pressing and releasing the button quickly will display the current sun rise and sun set times. Pressing the button again will enter the mode select menu. 

#include <Wire.h>
#include "RTClib.h" // from https://github.com/adafruit/RTClib
//#include <LiquidCrystal.h>
#include <LiquidCrystal_I2C.h>
#include <Encoder.h> // from http://www.pjrc.com/teensy/td_libs_Encoder.html
#include <EEPROM.h>

#define buton A0
#define releu A3

RTC_DS1307 RTC; // Tells the RTC library that we're using a DS1307 RTC
Encoder knob(2, 3); //encoder connected to pins 2 and 3 (and ground)
LiquidCrystal_I2C lcd(0x27,16,2); // I used an odd pin combination because I need pin 2 and 3 for the interrupts.

//the variables provide the holding values for the set clock routine

int setyeartemp; 
int setmonthtemp;
int setdaytemp;
int sethourstemp;
int setminstemp;
int setsecs = 0;
int maxday; // maximum number of days in the given month

int sethourstempon;
int setminstempon;
int sethourstempoff;
int setminstempoff;

// These variables are for the push button routine
int buttonstate = 0; //flag to see if the button has been pressed, used internal on the subroutine only
int pushlengthset = 3000; // value for a long push in mS
int pushlength = pushlengthset; // set default pushlength
int pushstart = 0;// sets default push value for the button going low
int pushstop = 0;// sets the default value for when the button goes back high

int knobval; // value for the rotation of the knob
boolean buttonflag = false; // default value for the button flag


void setup () {
    //Serial.begin(57600); //start debug serial interface
    Wire.begin(); //start I2C interface
    RTC.begin(); //start RTC interface
 
 //   lcd.begin(16,2); //Start LCD (defined as 16 x 2 characters)
    lcd.begin();
    lcd.backlight();
    lcd.clear(); 
    pinMode(buton,INPUT);//push button on encoder connected to A0 (and GND)
    digitalWrite(buton,HIGH); //Pull A0 high
    pinMode(releu,OUTPUT); //Relay connected to A3
    digitalWrite (releu, HIGH); //sets relay off (default condition)
    
    //Checks to see if the RTC is runnning, and if not, sets the time to the time this sketch was compiled.
    if (! RTC.isrunning()) {
    RTC.adjust(DateTime(__DATE__, __TIME__));
  }

sethourstempon = EEPROM.read(1);
setminstempon = EEPROM.read(2);
sethourstempoff = EEPROM.read(3);
setminstempoff = EEPROM.read(4);

 if (sethourstempon < 1) {
      sethourstempon = 0;
    }
 if (sethourstempon > 23) {
      sethourstempon = 23;
    }
 if (setminstempon < 0) {
      setminstempon = 0;
    }
 if (setminstempon > 59) {
      setminstempon = 59;
    }

 if (sethourstempoff < 1) {
      sethourstempoff = 0;
    }
 if (sethourstempoff > 23) {
      sethourstempoff = 23;
    }
 if (setminstempoff < 0) {
      setminstempoff = 0;
    }
 if (setminstempoff > 59) {
      setminstempoff = 59;
    }
}
           

void loop () {
   
    DateTime now = RTC.now(); //get time from RTC
    //Display current time
    lcd.setCursor (0,0);
    if (now.day() <10) 
      {
        lcd.print("0");
      }
    lcd.print(now.day(), DEC);
    lcd.print('/');
    if (now.month() <10) 
      {
        lcd.print("0");
      }
    lcd.print(now.month());
    lcd.print('/');
    lcd.print(now.year(), DEC);
    lcd.print(" ");
    lcd.setCursor (0,1);
    if (now.hour() <10) 
      {
        lcd.print("0");
      }
    lcd.print(now.hour(), DEC);
    lcd.print(':');
    if (now.minute() <10) 
      {
        lcd.print("0");
      }
    lcd.print(now.minute(), DEC);
    lcd.print(':');
    if (now.second() <10) 
      {
        lcd.print("0");
      }
    lcd.print(now.second());
//    lcd.print("     ");
    
if ((now.hour() == sethourstempon) and (now.minute() == setminstempon))  // sethourstempon
      {
        digitalWrite(releu, LOW);
        lcd.setCursor (9,1);
        lcd.print("*");
      }
if ((now.hour() == sethourstempoff) and (now.minute() == setminstempoff))
     {
        digitalWrite(releu, HIGH);
        lcd.setCursor (9,1);
        lcd.print(" ");
      }

lcd.setCursor (11,0);
 if (sethourstempon <10) 
      {
        lcd.print("0");
      }
    lcd.print(sethourstempon);
    lcd.print(':');
    if (setminson <10) 
      {
        lcd.print("0");
      }
    lcd.print(setminstempon);
lcd.setCursor (11,1);
 if (sethourstempoff <10) 
      {
        lcd.print("0");
      }
    lcd.print(sethourstempoff);
    lcd.print(':');
    if (setminsoff <10) 
      {
        lcd.print("0");
      }
    lcd.print(setminstempoff);

    
    pushlength = pushlengthset;
    pushlength = getpushlength ();
    delay (10);
    
    if (pushlength <pushlengthset) {     
      ShortPush ();   
    }
    
       
       //This runs the setclock routine if the knob is pushed for a long time
       if (pushlength > pushlengthset) {
         lcd.clear();
         DateTime now = RTC.now();
         setyeartemp=now.year(),DEC;
         setmonthtemp=now.month(),DEC;
         setdaytemp=now.day(),DEC;
         sethourstemp=now.hour(),DEC;
         setminstemp=now.minute(),DEC;
         setclock();
         pushlength = pushlengthset;
       };
}

//sets the clock
void setclock (){
   setyear ();
   lcd.clear ();
   setmonth ();
   lcd.clear ();
   setday ();
   lcd.clear ();
   sethours ();
   lcd.clear ();
   setmins ();
   lcd.clear();
   sethourson ();
   lcd.clear ();
   setminson ();
   lcd.clear();
   sethoursoff ();
   lcd.clear ();
   setminsoff ();
   lcd.clear();
  
   RTC.adjust(DateTime(setyeartemp,setmonthtemp,setdaytemp,sethourstemp,setminstemp,setsecs));
   EEPROM.write(1, sethourstempon);
   EEPROM.write(2, setminstempon); 
   EEPROM.write(3, sethourstempoff);
   EEPROM.write(4, setminstempoff);
    
   delay (1000);
   
}

// subroutine to return the length of the button push.
int getpushlength () {
  buttonstate = digitalRead(buton);  
       if(buttonstate == LOW && buttonflag==false) {     
              pushstart = millis();
              buttonflag = true;
          };
          
       if (buttonstate == HIGH && buttonflag==true) {
         pushstop = millis ();
         pushlength = pushstop - pushstart;
         buttonflag = false;
       };
       return pushlength;
}

// The following subroutines set the individual clock parameters
int setyear () {
//lcd.clear();
    lcd.setCursor (0,0);
    lcd.print ("Set Year");
    pushlength = pushlengthset;
    pushlength = getpushlength ();
    if (pushlength != pushlengthset) {
      return setyeartemp;
    }

    lcd.setCursor (0,1);
    knob.write(0);
    delay (50);
    knobval=knob.read();
    if (knobval < -1) { //bit of software de-bounce
      knobval = -1;
    }
    if (knobval > 1) {
      knobval = 1;
    }
    setyeartemp=setyeartemp + knobval;
    if (setyeartemp < 2014) { //Year can't be older than currently, it's not a time machine.
      setyeartemp = 2014;
    }
    lcd.print (setyeartemp);
    lcd.print("  "); 
    setyear();
}
  
int setmonth () {
//lcd.clear();
   lcd.setCursor (0,0);
    lcd.print ("Set Month");
    pushlength = pushlengthset;
    pushlength = getpushlength ();
    if (pushlength != pushlengthset) {
      return setmonthtemp;
    }

    lcd.setCursor (0,1);
    knob.write(0);
    delay (50);
    knobval=knob.read();
    if (knobval < -1) {
      knobval = -1;
    }
    if (knobval > 1) {
      knobval = 1;
    }
    setmonthtemp=setmonthtemp + knobval;
    if (setmonthtemp < 1) {// month must be between 1 and 12
      setmonthtemp = 1;
    }
    if (setmonthtemp > 12) {
      setmonthtemp=12;
    }
    lcd.print (setmonthtemp);
    lcd.print("  "); 
    setmonth();
}

int setday () {
  if (setmonthtemp == 4 || setmonthtemp == 5 || setmonthtemp == 9 || setmonthtemp == 11) { //30 days hath September, April June and November
    maxday = 30;
  }
  else {
  maxday = 31; //... all the others have 31
  }
  if (setmonthtemp ==2 && setyeartemp % 4 ==0) { //... Except February alone, and that has 28 days clear, and 29 in a leap year.
    maxday = 29;
  }
  if (setmonthtemp ==2 && setyeartemp % 4 !=0) {
    maxday = 28;
  }
//lcd.clear();  
   lcd.setCursor (0,0);
    lcd.print ("Set Day");
    pushlength = pushlengthset;
    pushlength = getpushlength ();
    if (pushlength != pushlengthset) {
      return setdaytemp;
    }

    lcd.setCursor (0,1);
    knob.write(0);
    delay (50);
    knobval=knob.read();
    if (knobval < -1) {
      knobval = -1;
    }
    if (knobval > 1) {
      knobval = 1;
    }
    setdaytemp=setdaytemp+ knobval;
    if (setdaytemp < 1) {
      setdaytemp = 1;
    }
    if (setdaytemp > maxday) {
      setdaytemp = maxday;
    }
    lcd.print (setdaytemp);
    lcd.print("  "); 
    setday();
}

int sethours () {
//lcd.clear();
    lcd.setCursor (0,0);
    lcd.print ("Set Hours");
    pushlength = pushlengthset;
    pushlength = getpushlength ();
    if (pushlength != pushlengthset) {
      return sethourstemp;
    }

    lcd.setCursor (0,1);
    knob.write(0);
    delay (50);
    knobval=knob.read();
    if (knobval < -1) {
      knobval = -1;
    }
    if (knobval > 1) {
      knobval = 1;
    }
    sethourstemp=sethourstemp + knobval;
    if (sethourstemp < 1) {
      sethourstemp = 0;
    }
    if (sethourstemp > 23) {
      sethourstemp=23;
    }
    lcd.print (sethourstemp);
    lcd.print("  "); 
    sethours();
}

int setmins () {
//lcd.clear();
   lcd.setCursor (0,0);
    lcd.print ("Set Mins");
    pushlength = pushlengthset;
    pushlength = getpushlength ();
    if (pushlength != pushlengthset) {
      return setminstemp;
    }

    lcd.setCursor (0,1);
    knob.write(0);
    delay (50);
    knobval=knob.read();
    if (knobval < -1) {
      knobval = -1;
    }
    if (knobval > 1) {
      knobval = 1;
    }
    setminstemp=setminstemp + knobval;
    if (setminstemp < 0) {
      setminstemp = 0;
    }
    if (setminstemp > 59) {
      setminstemp=59;
    }
    lcd.print (setminstemp);
    lcd.print("  "); 
    setmins();
}

int sethourson () {
//lcd.clear();
    lcd.setCursor (0,0);
    lcd.print ("Set Hours - ON");
    pushlength = pushlengthset;
    pushlength = getpushlength ();
    if (pushlength != pushlengthset) {
      return sethourstempon;
    }

    lcd.setCursor (0,1);
    knob.write(0);
    delay (50);
    knobval=knob.read();
    if (knobval < -1) {
      knobval = -1;
    }
    if (knobval > 1) {
      knobval = 1;
    }
    sethourstempon = sethourstempon + knobval;
    if (sethourstempon < 1) {
      sethourstempon = 0;
    }
    if (sethourstempon > 23) {
      sethourstempon = 23;
    }
    lcd.print (sethourstempon);
    lcd.print("  "); 
    sethourson();
}

int setminson () {
//lcd.clear();
   lcd.setCursor (0,0);
    lcd.print ("Set Mins - ON");
    pushlength = pushlengthset;
    pushlength = getpushlength ();
    if (pushlength != pushlengthset) {
      return setminstempon;
    }

    lcd.setCursor (0,1);
    knob.write(0);
    delay (50);
    knobval=knob.read();
    if (knobval < -1) {
      knobval = -1;
    }
    if (knobval > 1) {
      knobval = 1;
    }
    setminstempon = setminstempon + knobval;
    if (setminstempon < 0) {
      setminstempon = 0;
    }
    if (setminstempon > 59) {
      setminstempon = 59;
    }
    lcd.print (setminstempon);
    lcd.print("  "); 
    setminson();
}

int sethoursoff () {
//lcd.clear();
    lcd.setCursor (0,0);
    lcd.print ("Set Hours - OFF");
    pushlength = pushlengthset;
    pushlength = getpushlength ();
    if (pushlength != pushlengthset) {
      return sethourstempoff;
    }

    lcd.setCursor (0,1);
    knob.write(0);
    delay (50);
    knobval=knob.read();
    if (knobval < -1) {
      knobval = -1;
    }
    if (knobval > 1) {
      knobval = 1;
    }
    sethourstempoff = sethourstempoff + knobval;
    if (sethourstempoff < 1) {
      sethourstempoff = 0;
    }
    if (sethourstempoff > 23) {
      sethourstempoff = 23;
    }
    lcd.print (sethourstempoff);
    lcd.print("  "); 
    sethoursoff();
}

int setminsoff () {
//lcd.clear();
   lcd.setCursor (0,0);
    lcd.print ("Set Mins - OFF");
    pushlength = pushlengthset;
    pushlength = getpushlength ();
    if (pushlength != pushlengthset) {
      return setminstempoff;
    }

    lcd.setCursor (0,1);
    knob.write(0);
    delay (50);
    knobval=knob.read();
    if (knobval < -1) {
      knobval = -1;
    }
    if (knobval > 1) {
      knobval = 1;
    }
    setminstempoff = setminstempoff + knobval;
    if (setminstempoff < 0) {
      setminstempoff = 0;
    }
    if (setminstempoff > 59) {
      setminstempoff = 59;
    }
    lcd.print (setminstempoff);
    lcd.print("  "); 
    setminsoff();
}


void ShortPush () {
  //This displays the calculated sunrise and sunset times when the knob is pushed for a short time.
lcd.clear();
for (long Counter = 0; Counter < 604 ; Counter ++) { //returns to the main loop if it's been run 604 times 
                                                     //(don't ask me why I've set 604,it seemed like a good number)
  lcd.setCursor (0,0);
  lcd.print ("Sunrise ");
 
    
  //If the knob is pushed again, enter the mode set menu
  pushlength = pushlengthset;
  pushlength = getpushlength ();
  if (pushlength != pushlengthset) {
    lcd.clear ();
    lcd.print("new push?");
  }
  
}
lcd.clear();
}
