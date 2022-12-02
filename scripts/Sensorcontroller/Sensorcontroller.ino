//Changelog:
//0.2 - Replaced placeholder helptext with actual command list, added comments to source code
//0.3 - Enables the watchdog timer, adds version command, improves reading strings from program memory
//0.4 - Changed to single input point with default input format
#include <avr/wdt.h>
#define compPinput A0 // compressor pressure input
#define oilinput A1 //oil temp input pin
#define commandlengthmax 256 //length of input buffer
#define adcmax = 1024 //resolution of ADC
const char rev[] = "0.4";
const char PROGMEM startuptext[] = "Enter a command followed by <Enter>. Send 'help' for a list of commands. Clear command buffer with <Enter>",
helptext[] = " digread Pin#      - Read value of digital pin\n\
 digwrite Pin# 0/1 - Write value to digital pin \n\
 help              - Display helptext \n\
 loopback          - Enable/disable serial loopback \n\
 readadc Pin#      - Read raw value from ADC pin (0-1024) \n\
 oiltemp           - Read oil temperature in Celsius \n\
 compress          - Read compressure pressure\n\
 version           - Display software version \n ";
int commandindex =0, looptoggle = 0;
char commandbuffer[commandlengthmax];
int praw; // variables to read in values
float pval; // variables to read in values
   
void setup() {
  Serial.begin(19200);
  Serial.println("Setup Complete");
  //strcpy_P(bigstring, (char*)pgm_read_word(&(stringindex[0]))); //read startup text into bigstring
  //strcpy_P(bigstring, pgm_read_word(&startuptext));
  Serial.println((__FlashStringHelper *)startuptext);
  wdt_enable(WDTO_1S);
  pinMode(13, OUTPUT); // to make it possible to toogle built-in LED
}//end setup
     
//function to correct adc
//Not needed for arduino nano, incomplete and left over from ESP32
//float adccorrected(int adcinput) {
//            int reading, indexapprox;
//            float readingfloat, out;
//               
//            reading = analogRead(adcinput);
//            Serial.print("RAW ADC: ");
//            Serial.println (reading);
//            readingfloat = reading;
//            indexapprox = (readingfloat/4096) * 60;  
//            out = readingfloat;
//                  return(out);
//        }//ADC correction function ends here

//print help file into Serial
void help(){
            Serial.println((__FlashStringHelper *)helptext);  
}//end help file

// Get next integer from string, return 1 if not found
// moves the cmdindex to the end 
int gotint(char** cmdindex, int * value){
  // skip over spaces
  while((*cmdindex)[0] == ' ' || (*cmdindex)[0] == '\t'){
    (*cmdindex)++;
  }
  if(isdigit((*cmdindex)[0])){
    // set the value
    *value = atoi(*cmdindex);
    // advance cmdindex to the next sace of end of string
    while((*cmdindex)[0] != ' ' && (*cmdindex)[0] != '\t' && (*cmdindex)[0] > 0 ){
      (*cmdindex)++; 
    }
    return 1;
  } else {
    return 0;
  }
}

//Read raw value from ADC
void rawadc(char* cmdindex){
  int pin = 0;
  if( !gotint( &cmdindex, &pin)){
    Serial.println("ReadADC: Missing Pin Number");
    return 0;
  }
  if( pin<0 || pin>7 ){//Make sure pin is valid
    Serial.println("ReadADC: Invalid pin");
    return 0;
  }
  Serial.println(analogRead(pin));  //Print ADC value of pin          
  wdt_reset();
}//end raw ADC reading

//Read digital value
void digiread(char* cmdindex){
  int pin = 0;
  if( !gotint( &cmdindex, &pin)){
    Serial.println("DigRead: Missing Pin Number");
    return 0;
  }
  if( pin<2 || pin>13 ){//Make sure pin is valid
    Serial.println("DigRead: Invalid pin");
    return 0;
  }
  Serial.println(digitalRead(pin));  //Print value of digital pin to serial       
  wdt_reset();
}//end digitalread

//Write digital value
void digiwrite(char* cmdindex){
  int pin = 0, val=0;
  if( !gotint( &cmdindex, &pin)){
    Serial.println("DigWrite: Missing Pin Number");
    return 0;
  }
  if( pin<2 || pin>13 ){//Make sure pin is valid
    Serial.println("DigWrite: Invalid pin");
    return 0;
  }
  if( !gotint( &cmdindex, &val)){
    Serial.println("DigWrite: Missing Value");
    return 0;
  }
  if(val<0 || val>1){//Is the input 0 or 1?
    Serial.println("DigWrite: Invalid value"); 
    return 0;                           
  } 
  switch(val){
    case 1://do this if you're writing HIGH
    digitalWrite(pin,HIGH);
    Serial.print("Wrote pin ");
    Serial.print(pin);
    Serial.println(" HIGH");
    return 0;
    break;

    case 0://Do this instead if you're writing LOW
    digitalWrite(pin,LOW);
    Serial.print("Wrote pin ");
    Serial.print(pin);
    Serial.println(" LOW");
    return 0;
    break;
  }
  wdt_reset();
}//end digiwrite

void loopback(int in){ //SERIAL LOOPBACK
          char out; 
          out = in;
          Serial.print(out);  
}//end loopback


int waitforcommand(char inputcommand[], int *index){//Takes incoming characters and puts them into the buffer
  char incoming;
                          
  if(Serial.available()){ //triggers on incoming serial
    incoming = Serial.read();
    switch(looptoggle) //This runs if loopback is enabled and prints the incoming character
      case 1:
        Serial.print(incoming);
    if( incoming == '\n' || incoming == '\r' || *index == commandlengthmax-1 ) { // if <cr> <lf> or length exceeded
      if(*index) { // also check if command length > 0 
        inputcommand[*index] = 0; // terminate the string
        *index = 0; //Sets index to zero, so next command is entered at start of inputcommand[]
        return 1; //Set return value to 1
      } else {
        return 0; // new line at zero command length -> just continue
      }
    } else { //Stores characters in the command buffer and increases index
      inputcommand[(*index)++] = incoming;
      return 0; //Returns zero since <cr> or <lf> wasn't received
    }
  } //end incoming serial check
} //end wait for command

//Interpret command
void readcommand(char* (inputcmd)){
  wdt_reset();
  //Prints the string that gets passed to this command, uncomment if things break
//          Serial.print(inputcmd);
//          Serial.println(" THIS IS WHAT THE READCOMMAND FUNCTION SEES");

          
          // Compares input to list of commands. The format is:
          //    commandname par1 par2
          // The parameters are optional (depending on the command)
          // Any number of spaces/tabs >0 possible betfore the parameters.
          // If you want to add a command to this function, do it with "else if(strcmp(inputcmd, "YOURCOMMAND")==0)"
          // There are probably numerous better ways to write this function but this was written after
          // I hadn't written C for two years and it working at all is an accomplishment - and it WORKS.

          // Find end of command name
          char* cmdindex;
          for(cmdindex = inputcmd;
        	   cmdindex[0] != ' ' && cmdindex[0] != '\t' && cmdindex[0] > 0 &&
        	     cmdindex-inputcmd < commandlengthmax-1;
        	   cmdindex++){
          }
          // Terminate intputcmd and set cmdindex to next character
          if( cmdindex[0] > 0 ){
        	  cmdindex[0] = 0;
        	  cmdindex++;
          }
          //sus test input
         if(strcmp(inputcmd, "sus")==0){
            Serial.println("Sugoma");
                  }//SUGOMA//

          //digital write command  
          else if(strcmp(inputcmd, "digwrite")==0){//Command to write digital pin
            digiwrite(cmdindex);
          }    

          //help command
          else if(strcmp(inputcmd, "help")==0){//Command to print help into console
            help();
           }//help//

          //Raw ADC input
          else if(strcmp(inputcmd, "readadc")==0){//Command to read the raw value from the ADC
            rawadc(cmdindex);
           }//Raw ADC input

          
          else if(strcmp(inputcmd, "digread")==0){//Command to read the value of a digital pin
            digiread(cmdindex);
           }
          
          //oil temp read
          //Does nothing, finish when temp sensor arrives
          else if(strcmp(inputcmd, "oiltemp")==0){//Command to read oil temp in degrees celsius
            Serial.print("Oil Temp pin raw input:");
            Serial.println(analogRead(oilinput));
//            Serial.println(adccorrected(oilinput));           
          }//end oil temp command
          
          //compressor pressure read
          else if(strcmp(inputcmd, "compress")==0){//Command to read oil temp in degrees celsius
            Serial.print("Compressor pressure raw input: ");
            praw = analogRead(compPinput);
            Serial.print(praw);
            pval = 5.0*praw/1023;
            Serial.print(" ~ ");
            Serial.print(pval);
            pval = 300.0*(pval-0.45)/4.0;
            Serial.print("V ~ ");
            Serial.print(pval);
            Serial.println("Psi");
//            Serial.println(adccorrected(oilinput));           
          }//end oil temp command

         //prints version
          else if(strcmp(inputcmd, "version")==0){
         Serial.println(rev);
          }//end version
          
          //enable/disable serial loopback
          else if(strcmp(inputcmd, "loopback")==0){
          looptoggle = looptoggle ^ 1; //Toggle loopback var between 0 and 1
          switch(looptoggle){
          case 1:
          Serial.println("Loopback ON");
          break;
          
          case 0:
          Serial.println("Loopback OFF");
          break;
          }
          }//end loopback command
          
          else{ //Invalid command response
            Serial.print("Invalid command: ");
            Serial.println(inputcmd);
               }
          //End else ifs for command
}//end command interpreter

void loop() {
  //begin loop  
  while(Serial.available()){
    if(waitforcommand(commandbuffer, &commandindex) == 1){
      readcommand((commandbuffer));      
    }
  }
  wdt_reset();
}//end loop
 
 
