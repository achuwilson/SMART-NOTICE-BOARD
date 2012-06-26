
#include <NewSoftSerial.h>


String present_msg, past_msg;
NewSoftSerial mySerial(2, 3);

char my_string[150];
char *stringptr=my_string;
int i,d;
char inchar;
char fl;

void setup()  
{
  
  


delay(40000);
delay(10000);
Serial.begin(19200);
mySerial.begin(19200);
Serial.println('A');			// for baudrate synchronization we hav to sent 'A' first
delay(2000);
Serial.println("AT");			//initialize AT commands
delay(500);
Serial.println("ATE0");			// turn echo off
delay(500);
Serial.println("AT+CMGDA=\"DEL ALL\"");	// Delete the unread messages
delay(500);
mySerial.println("WELCOME");		// print welcome



}

void loop()                     // run over and over again
{
  
 if (Serial.available()>0)
 
 {
   i=0;
  d=0;
 
   inchar=Serial.read();
   if(inchar=='+')
         {
         delay(10);
         inchar=Serial.read();
         if(inchar=='C')
           {
             delay(10);
             inchar=Serial.read();
                 if(inchar=='M')
                   {
                     delay(10);
                     inchar=Serial.read();
                     if(inchar=='T')
                     {
                       delay(10);
                       while(Serial.available()>0)
                       {
                        Serial.read();
                       }
                       //mySerial.println("got msg");
                       mySerial.println("                                         ");
                       delay(500);
                       Serial.println("AT+CMGR=1");				// read the message
                       delay(3000);
                       i=0;
                        while(Serial.available()>3)
  
                           { 
                                   while(i<61)
                                     {
                                   Serial.read();      // flush out unnecessary data
                                     i++;
                                       }
                                     Serial.read();
                                   Serial.read();  
                               inchar=Serial.read();
                               
                               if(inchar=='1')			//check 1st byte of password
                                 {
                                   delay(10);
                                     inchar=Serial.read();
                                    
                                       if(inchar=='2')			//check 2nd byte of password
                                         {
                                           
                                                   
                                               delay(10);
                                                       inchar=Serial.read();
                                                         if(inchar=='3')	//check 3rd byte of password
                                                       
                                                                     {
                                                                      while(Serial.available()>3)
                                                                     { 
                                                                     fl=Serial.read();
           
                                                                       mySerial.print(fl);
                                                                     }
                                                                   }
                                           
                                         }
                                   
                                 }
                               
                             }
                             
                             delay(200);
                             Serial.println("AT+CMGDA=\"DEL ALL\"");
                             delay(500);
                             Serial.read();
                             delay(100);
                       
                       
                     }
                   }
             
         
           }
          
 
           
           
         }
   
   
 }
}
       
