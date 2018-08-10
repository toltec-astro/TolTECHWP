/*
  Quadtest

  Generates a quadrature signal. Features
  - Make quadrature signal out of quadA, quadB outputs
  - Count up number of steps or 10, 100, 1000, 10000
 */

#define quadA A4 // Quadrature A pin
#define quadB A5 // Quadrature B pin
#define butpin A0 // Button input pin
#define ledpin 13 // LED to indicate button steps
#define stepin A3 // output for button steps
#define butdiv 8000 // Number of loops until button count increases
#define quadiv 4 // Number of quadcnt steps for new
#define serial 0 // set >0 for serial output
#define rndm 256 // random variables for Linear Congruential Generator
#define rndc 110 //       rand[i+1] = ( a * rand[i] + c ) % m
#define rnda  69 // Set to 69 - set rnda = 0 to deactivate "random" noise

#include "Arduino.h"

int butcnt =  0; // >0 when button is pressed, <0 when clock is running
int butstep = 0; // Button step counts (counts up while button is pressed)
int quadcnt = 0; // quadrature step counter
int rand = 0; // random variable goes through every even number 0..255

// the setup function runs once when you press reset or power the board
void setup() {
  // initialize pins as input or outputs.
  pinMode(quadA, OUTPUT);
  pinMode(quadB, OUTPUT);
  pinMode(butpin, INPUT_PULLUP);
  pinMode(ledpin, OUTPUT);
  if(serial) Serial.begin(9600);
}

// the loop function runs over and over again forever (Result: 106.6kHz for simple on/off)
void loop() {
	// Update quad outputs
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
    	if(butcnt == 4*butdiv){
    		butcnt = 1;
    		butstep +=1;
    	}
    	// Blink LED in first quarter of button count
    	if( butcnt < butdiv ){
    		digitalWrite(ledpin, LOW);
    		digitalWrite(stepin, LOW);
    	} else {
    		digitalWrite(ledpin, HIGH);
    		digitalWrite(stepin, HIGH);
    	}
    } else {
    	// ** Button is released
    	digitalWrite(ledpin, LOW);
    	digitalWrite(stepin, LOW);
    	if(butcnt > 0){
    		if( butstep > 4) { butcnt = -butstep;
    		} else if ( butstep > 3 ) { butcnt = -10000;
    		} else if ( butstep > 2 ) { butcnt = -1000;
    		} else if ( butstep > 1 ) { butcnt = -100;
    		} else if ( butstep > 0 ) { butcnt = -10; }
    	}
    }
    // Update quadrature counter from button
    if( butcnt < 0 ) {
    	quadcnt++;
    	butcnt++;
    }
    // Update quadrature counter from random
    rand = ( rnda * rand + rndc ) % rndm;
    if ( rand > 256-32-3 ) { quadcnt++; }
    if ( rand < 32+1  )    { quadcnt--; }
    // Update serial output
    if( serial ){
    	Serial.print(butstep);
    	Serial.print("  ");
    	Serial.println(butcnt);
    }
}
