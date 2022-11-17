//Changelog:
//0.2 - Replaced placeholder helptext with actual command list, added comments to source code
//0.3 - Enables the watchdog timer, adds version command, improves reading strings from program memory
#include <avr/wdt.h>
#define compPinput A0 // compressor pressure input
#define oilinput A1 //oil temp input pin
#define commandlength 9 //length of input buffer
#define adcmax = 1024 //resolution of ADC
const char rev[] = "0.3";
const char PROGMEM startuptext[] = "Enter a command followed by #. Send help# for a list of commands. Clear command buffer with #",
helptext[] = " digread  - Read value of digital pin\n\
 digwrite - Write value to digital pin \n\
 help     - Display helptext \n\
 loopback - Enable/disable serial loopback \n\
 readadc  - Read raw value from ADC pin (0-1024) \n\
 oiltemp  - Read oil temperature in Celsius \n\
 compress - Read compressure pressure\n\
 version  - Display software version \n ";
int commandindex =0, looptoggle = 0;
char commandbuffer[commandlength];
int praw; // variables to read in values
float pval; // variables to read in values
   
void setup() {
  for(int i=0;i<commandlength - 1;i++)
  {
    commandbuffer[i] = ' '; //Fill command buffer with spaces
  }
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

//Fills the command buffer with spaces       
void clearbuffer(){
  for(int i=0;i<commandlength -1 ;i++)
       {
           commandbuffer[i] = ' ';
        }
}//end clear command buffer

//print help file into Serial
void help(){
            Serial.println((__FlashStringHelper *)helptext);  
}//end help file

//Read raw value from ADC
void rawadc(){
            int pin = 0;
            Serial.println("Enter ADC pin (0-7):");
            for(int w = 0; w == 0;){
              if (Serial.available()){
              pin = Serial.read() - 48; //Subtract 48 to turn ASCII into pin #
              w=1;
              }
              wdt_reset();
            }
              if(pin<0 || pin>7){//Make sure pin is valid
                Serial.println("Invalid pin");                            
              }
             else{
              Serial.println(analogRead(pin));  //Print ADC value of pin          
             }
}//end raw ADC reading

//Read digital value
void digiread(){
            int pin = 0;
            Serial.println("Enter digital pin (2-13):");
            for(int w = 0; w == 0;){ //begin loop that waits for input
              Serial.setTimeout(50); //Keeps parseInt() from stopping the program for 1000ms
              if (Serial.available()){
              pin = Serial.parseInt();
              w=1; //exit input loop
              }
             wdt_reset(); 
            }
              if(pin<2 || pin>13){//is the pin in range?
                Serial.println("Invalid pin");                            
              }
             else{
              Serial.println(digitalRead(pin));  //Print value of digital pin to serial          
             }
}//end digitalread

//Write digital value
void digiwrite(){
            int pin = 0, val=0;
            Serial.println("Enter digital pin (2-13):");
            for(int w = 0; w == 0;){//input checking loop begins here
              Serial.setTimeout(50); //Keeps parseint from holding the program hostage for 1000ms
              if (Serial.available()){
              pin = Serial.parseInt();
              w=1;//input checking loop ends here
              }     
              wdt_reset();         
            }
              if(pin<2 || pin>13){//Is the pin valid?
                Serial.println("Invalid pin"); 
                return(0);                           
              }
              Serial.println("Enter value to write (0,1):");

                 for(int w = 0; w == 0;){//begin input checking loop
              Serial.setTimeout(50);
              if (Serial.available()){
              val = Serial.parseInt();
              w=1;
              }    //end input checking loop                        
             
}
  if(val<0 || val>1){//Is the input 0 or 1?
                Serial.println("Invalid value"); 
                return(0);                           
              } 
          switch(val){
            case 1://do this if you're writing HIGH
            digitalWrite(pin,HIGH);
            Serial.print("Wrote pin ");
            Serial.print(pin);
            Serial.println(" HIGH");
            return(0);
            break;

            case 0://Do this instead if you're writing LOW
            digitalWrite(pin,LOW);
            Serial.print("Wrote pin ");
            Serial.print(pin);
            Serial.println(" LOW");
            return(0);
            break;
          }
}//end digiwrite

void loopback(int in){ //SERIAL LOOPBACK
          char out; 
          out = in;
          Serial.print(out);  
}//end loopback


int waitforcommand(char inputcommand[], int *index){//Takes incoming characters and puts them into the buffer
  char incoming;
  if(*index >= commandlength -1){//Keeps the index within the boundaries of the command buffer
    *index = 0;
  }     
                          
  if(Serial.available()){//triggers on incoming serial
    incoming = Serial.read();
    switch(looptoggle)//This runs if loopback is enabled and prints the incoming character
      case 1:
        Serial.print(incoming);
    if(incoming == '#'){ //# signifies the end of a command
      *index = 0; //Sets index to zero, so next command is entered at start of inputcommand[]
      return 1; //Set return value to 1
    } else if(incoming != '\n' && incoming != '\t' && incoming != '\r'){ //Stores characters in the command buffer if they're not #
      inputcommand[*index] = incoming;
      *index = (*index + 1);
      return 0;//Returns zero since # wasn't received
    } else {
      return 0;
    }
  }//end incoming serial loop 
} //end wait for command

//Interpret command
 void readcommand(char* (inputcmd)){
  wdt_reset();
  //Prints the string that gets passed to this command, uncomment if things break
//          Serial.print(inputcmd);
//          Serial.println(" THIS IS WHAT THE READCOMMAND FUNCTION SEES");

          
          //Compares input to list of commands
          //Commands are 8 characters max. Commands less than 8 characters are sent to this function with extra spaces
          //Example: entering "sus#" via serial results in a command buffer that reads "sus     ".
          //If you want to add a command to this function, do it with "else if(strcmp(inputcmd, "YOURCOMMAND + enough spaces to pad this string out to 8 characters total")==0"
          //There are probably numerous better ways to write this function but this was written after I hadn't written C for two years and it working at all is an accomplishment

          
          //sus test input
         if(strcmp(inputcmd, "sus     ")==0){
            Serial.println("Sugoma");
                  }//SUGOMA//

          //digital write command  
          else if(strcmp(inputcmd, "digwrite")==0){//Command to write digital pin
            digiwrite();              
          }    

          //help command
          else if(strcmp(inputcmd, "help    ")==0){//Command to print help into console
            help();
           }//help//

          //Raw ADC input
          else if(strcmp(inputcmd, "readadc ")==0){//Command to read the raw value from the ADC
            rawadc();
           }//Raw ADC input

          
          else if(strcmp(inputcmd, "digread ")==0){//Command to read the value of a digital pin
            digiread();
           }
          
          //oil temp read
          //Does nothing, finish when temp sensor arrives
          else if(strcmp(inputcmd, "oiltemp ")==0){//Command to read oil temp in degrees celsius
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
          else if(strcmp(inputcmd, "version ")==0){
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
          clearbuffer(); //Empty out command buffer
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
 
 
