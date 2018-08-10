   /*
  Quadtest

  Generates a quadrature signal. Features
  - Make quadrature signal out of quadA, quadB outputs
  - Has internal counter (quadcnt) which is quadiv * quadrature steps
  - Count up 10 (1 beep), 100 (2 beep), 1000 (3 beep), 10000 (4 beep) steps
    or just N steps (N >= 4 beeps). Press button and releast at the N'th beep
  - Optional noise (only use with quadiv > 3)
  - Has pps simulator
 */

#define quadA A4 // Quadrature A pin
#define quadB A5 // Quadrature B pin
#define butpin A0 // Button input pin for Quadrature counter
#define ledpin 13 // LED to indicate button steps
#define ppspin A3 // output for PPS simulator (uses millis)
#define butdiv 20000 // Number of loops until button count increases
#define quadiv 3 // Number of quadcnt steps for new
#define serial 0 // set >0 for serial output (needs butdiv ~ 20, else butdiv ~ 20000)
#define rndm 256 // random variables for Linear Congruential Generator
#define rndc 110 //       rand[i+1] = ( a * rand[i] + c ) % m
#define rnda  69 // Set to 69 - set rnda = 0 to deactivate "random" noise

#include "Arduino.h"

long butcnt =  0; // >0 and increasing when button is pressed, <0 and increasing when clock is running
long butstep = 0; // Button step counts (counts up while button is pressed)
long quadcnt = 0; // quadrature step counter
int rndv = 0;     // random variable goes through every even number 0..255

// the setup function runs once when you press reset or power the board
void setup() {
  // initialize pins as input or outputs.
  pinMode(quadA, OUTPUT);
  pinMode(quadB, OUTPUT);
  pinMode(butpin, INPUT_PULLUP);
  pinMode(ledpin, OUTPUT);
  pinMode(ppspin, OUTPUT);
  if(serial) Serial.begin(9600);
}

// the loop function runs over and over again forever (Result: 106.6kHz for simple on/off)
void loop() {
	// Update quadrature outputs
    if((quadcnt/quadiv)&2){
    	if((quadcnt/quadiv)&1){ // 11
    		digitalWrite(quadA, HIGH);
    		digitalWrite(quadB, HIGH);
    	} else { // 10
    		digitalWrite(quadA, HIGH);
    		digitalWrite(quadB, LOW);
    	}
    } else {
    	if((quadcnt/quadiv)&1){ // 01
    		digitalWrite(quadA, LOW);
    		digitalWrite(quadB, LOW);
    	} else { // 00
    		digitalWrite(quadA, LOW);
    		digitalWrite(quadB, HIGH);
    	}
    }
    // Check button intput
    if( digitalRead(butpin) == LOW){
    	// ** Button is pressed
    	// Initialize button press if it was unpressed
    	if(butcnt < 1){
    		butcnt = 1;
    		butstep = 1;
    	// else increase button count
    	} else butcnt++;
    	// Increase count if button count is reached
    	if(butcnt == butdiv){
    		butcnt = 1;
    		butstep +=1;
    	}
    	// Blink LED in last half of button count
    	if( butcnt < butdiv / 2 ){
    		digitalWrite(ledpin, LOW);
    	} else {
    		digitalWrite(ledpin, HIGH);
    	}
    } else {
    	// ** Button is released
    	digitalWrite(ledpin, LOW);
    	if(butcnt > 0){
    		if( butstep > 4) { butcnt = -butstep;
    		} else if ( butstep > 3 ) { butcnt = -10000*quadiv;
    		} else if ( butstep > 2 ) { butcnt = -1000*quadiv;
    		} else if ( butstep > 1 ) { butcnt = -100*quadiv;
    		} else if ( butstep > 0 ) { butcnt = -10*quadiv; }
    	}
    }
    // Update quadrature counter from button
    if( butcnt < 0 ) {
    	quadcnt++;
    	butcnt++;
    }
    // Update quadrature counter from random
    rndv = ( rnda * rndv + rndc ) % rndm;
    if ( rndv > 256-32-3-16 ) { quadcnt++; }
    if ( rndv < 32+1+16  )    { quadcnt--; }
    // Update pps simulator
    if( millis() % 1000 < 100 ){
      digitalWrite(ppspin, HIGH);
    } else {
      digitalWrite(ppspin, LOW);
    }
    // Update serial output
    if( serial ){
      Serial.print("ButStep = ");
    	Serial.print(butstep);
    	Serial.print("  ButCnt = ");
    	Serial.print(butcnt);
      Serial.print("  QuadCnt = ");
      Serial.print(quadcnt);
      Serial.print("  QuadOut = ");
      Serial.print(quadcnt/quadiv);
      Serial.print("  RndV = ");
      Serial.println(rndv);
    }
}
