
/*
*  This is the nega webswitch  version 1.5.1 for the remoteQth.com
*  If you need help, feel free to contact DM5XX@gmx.de
*  Sketch is developed with IDE Version 1.6.9 and later
*
*  This is free software. You can redistribute it and/or modify it under
*  the terms of Creative Commons Attribution 3.0 United States License if at least the version information says credits to remoteQTH.com :P
*
*  To view a copy of this license, visit http://creativecommons.org/licenses/by/3.0/us/
*  or send a letter to Creative Commons, 171 Second Street, Suite 300, San Francisco, California, 94105, USA.
*
*	Tribute to OL7M!
*  LLAP!
*/




#if ARDUINO > 18
#include <SPI.h>
#endif
#include <Ethernet.h>
#include <HashMap.h>
#include <Time.h>
#include <SD.h>
//#define FILLARRAY(a,n) a[0]=n, memcpy( ((char*)a)+sizeof(a[0]), a, sizeof(a)-sizeof(a[0]) );

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };  
byte ip[] = { 192, 168, 1, 177 };                      
byte gateway[] = { 192, 168, 1, 41 };                   
byte subnet[] = { 255, 255, 255, 0 };                  
EthernetServer server(80);                                  
String requestString;
File myFile;
String ajaxUrl;
String deviceIp;
String gatewayIp;
String title;
String jsUrl;
String jqueryUrl;
String cssUrl;
String faviconUrl;
String dotUrl;

// the Pins definition array. Remember this are the only pin numbers. allother array values belong to the indexs defined here.
byte pinsBank0[] = { 54, 2, 3, 5, 6, 7, 8, 9, 11, 12, 13, 14, 15, 16, 17, 18}; // remember do not use 0/1. 54 will be A0, 55 will be A1
byte pinsBank1[] = { 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34}; 
byte pinsBank2[] = { 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 53}; 
byte pinsBank3[] = { 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 1 };

// arrays to store the state if a button shoud be stay online even if its switched of aka "5s-ON"-Buttons
// everything is left as 16 part arrays. this means more ram, but less loops
boolean stayOnPinsBank0[16] =   { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
boolean stayOnPinsBank1[16] =   { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
boolean stayOnPinsBank2[16] =   { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
boolean stayOnPinsBank3[16] =	{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };

//arrays to define which buttons are long-on-button. startin with 0, setting 1 to the long switch buttons. must be 1:1 matched to the js buttons/description
boolean is5sPinBank0[] =        { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 }; // 3 in bank0
boolean is5sPinBank1[] =        { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 }; // 3 in bank1
boolean is5sPinBank2[] =        { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 }; // 6 in bank2
boolean is5sPinBank3[] =		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 }; // 0 in bank 3

boolean isOffPinBank0[] =       { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 }; // define if a button is a long-off-button. uses index from js file as everything else
boolean isOffPinBank1[] =       { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
boolean isOffPinBank2[] =       { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
boolean isOffPinBank3[] =		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };

byte indexOfOffPinPairBank0[] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 }; // definition which on-index the off-index belongs to
byte indexOfOffPinPairBank1[] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
byte indexOfOffPinPairBank2[] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
byte indexOfOffPinPairBank3[] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };

// push duration - Adjust the individual push duration of a "long push button" here - On & Off
byte pushDurationBank0[] =      { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
byte pushDurationBank1[] =      { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
byte pushDurationBank2[] =      { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
byte pushDurationBank3[] =		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };


// The long On Push -array contains the timestamp when the status should be cleared and the watchdog hits...
unsigned long longOnCleanUpTimeBank0[16];
unsigned long longOnCleanUpTimeBank1[16];
unsigned long longOnCleanUpTimeBank2[16];
unsigned long longOnCleanUpTimeBank3[16];

// The time the button is pushed - to be released after 1s. the time store is the time it has to be switched off and the watchdog hits, not the time is is added to the array!
unsigned long longOffCleanUpTimeBank0[16]; 
unsigned long longOffCleanUpTimeBank1[16]; 
unsigned long longOffCleanUpTimeBank2[16]; 
unsigned long longOffCleanUpTimeBank3[16];

unsigned long lastWatchdogRun;

String strpinsBank0;
String strpinsBank1;
String strpinsBank2;
String strpinsBank3;
String stris5sPinBank0;
String stris5sPinBank1;
String stris5sPinBank2;
String stris5sPinBank3;
String strisOffPinBank0;
String strisOffPinBank1;
String strisOffPinBank2;
String strisOffPinBank3;
String strindexOfOffPinPairBank0;
String strindexOfOffPinPairBank1;
String strindexOfOffPinPairBank2;
String strindexOfOffPinPairBank3;
String strpushDurationBank0;
String strpushDurationBank1;
String strpushDurationBank2;
String strpushDurationBank3;


void setup()
{  
  // set mode for all needed pins in each bank
  for(int out = 0; out < 16; out++)
  {
    pinMode(pinsBank0[out], OUTPUT);
    pinMode(pinsBank1[out], OUTPUT);
    pinMode(pinsBank2[out], OUTPUT);
	pinMode(pinsBank3[out], OUTPUT);
	digitalWrite(pinsBank0[out], LOW);
    digitalWrite(pinsBank1[out], LOW); 
    digitalWrite(pinsBank2[out], LOW); 
	digitalWrite(pinsBank3[out], LOW);
  }
  
  Serial.begin(115200);
  server.begin();          // Server starten
  
  if (!SD.begin(4)) {
     Serial.println("initialization failed!");
   return;
  }
  Serial.println("initialization done.");
  readSDSettings();
  initBanksFromSDCarsd();
  ip[0] = getStringPartByNr(deviceIp, '.', 0).toInt();
  ip[1] = getStringPartByNr(deviceIp, '.', 1).toInt();
  ip[2] = getStringPartByNr(deviceIp, '.', 2).toInt();
  ip[3] = getStringPartByNr(deviceIp, '.', 3).toInt();

  gateway[0] = getStringPartByNr(gatewayIp, '.', 0).toInt();
  gateway[1] = getStringPartByNr(gatewayIp, '.', 1).toInt();
  gateway[2] = getStringPartByNr(gatewayIp, '.', 2).toInt();
  gateway[3] = getStringPartByNr(gatewayIp, '.', 3).toInt();

  Ethernet.begin(mac, ip); // Client starten
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());
   
  adjustTime(10); // dont start with 0... think about buffer underruns of longs :P 
  lastWatchdogRun = now(); // set the whatchdog to current time. since there is no timesync, its second "10" in the year 0 (1970)

  //digitalWrite(56, HIGH); // use 53 still as an output - remember sd card operation may be disabled now... so this is a workaround.

}



/************************************************************************************************************************/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++ LOOP ++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/************************************************************************************************************************/
void loop()
{
  // the the watchdog hit once a second. dont waste performance by calling to often...
  if(lastWatchdogRun <= now()-1)
  {
    WatchdogBank0();
    WatchdogBank1();
    WatchdogBank2();
	WatchdogBank3();
	lastWatchdogRun = now();
  }
  
  // Create a client connection
  EthernetClient client = server.available();
  if (client) {
    while (client.connected()) {   
      if (client.available()) {
        char c = client.read();
     
        if (requestString.length() < 100) {
          requestString += c;
          //Serial.print(c);
         }

         //if HTTP request has ended
         if (c == '\n') {          
           int cmdSet = requestString.indexOf("Set/"); // see if its a set request
           int cmdGet = requestString.indexOf("Get/"); // see if its a get request
           int cmdGetAll = requestString.indexOf("GetAll"); // see if its a get request
           int cmdisLocal = requestString.indexOf("l"); // see if its a get request
           
           if(cmdSet >= 0)
           {
             byte currentBank = getStringPartByNr(requestString, '/', 2).toInt(); // the 2nd part is the bank-number
             String currentPinString = getStringPartByNr(requestString, '/', 3); // the 3nd part is the decimal-value to react on
             
             unsigned int currentDecimal = currentPinString.substring(0, currentPinString.indexOf(" HTT")).toInt(); // remove the _HTTP... and convert to int
             //Serial.println(currentDecimal);
             String theBinaryString = int2bin(currentDecimal); // convert the int to a binary string
             //Serial.println(theBinaryString);
             String revertedBinaryString = revertBinaryString(theBinaryString);  // revese the binarystring, because its kinda tight to left.. 
			 //Serial.println(revertedBinaryString);

             setPinsOfBank(currentBank, revertedBinaryString); // set the pins on arduino using the index
             SetPage(client, currentBank); // return the http status
           }

           if(cmdGetAll >= 0)
             GetAllPage(client);
           
           if(cmdGet >= 0)
           {
             byte currentBank = getStringPartByNr(requestString, '/', 2).toInt();
             GetPage(client, currentBank);
           }
           
           if(cmdSet < 0 && cmdGet < 0 && cmdGetAll < 0)
           {
             if(cmdisLocal >=0)
               MainPage(client, true);
             else
               MainPage(client, false);             
           }

           requestString="";  
  
           delay(1);
           client.stop();
         }
       }
    }
}
}

/**********************************************************************************************************************************/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ Watchgogs +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/**********************************************************************************************************************************/

void WatchdogBank0() // as im too lazy to use pointers, i copy pasted my code :P
{
    for(byte s = 0; s < 16; s++)
    {
      if(longOnCleanUpTimeBank0[s] > 0 && longOnCleanUpTimeBank0[s]<= now()) // default is 0 => only do something if there is something to do...
      {
        stayOnPinsBank0[s] = 1; // set the pin to permanent ON at index 
        digitalWrite(pinsBank0[s], LOW); // switch of this pin
        longOnCleanUpTimeBank0[s] = 0; // set its corresponding settingtime to default 0;
      }
      if(longOffCleanUpTimeBank0[s] > 0 && longOffCleanUpTimeBank0[s]<= now())
      {
        byte foundIndex = indexOfOffPinPairBank0[s];
        stayOnPinsBank0[foundIndex] = 0; // set the pin to OFF at index
        digitalWrite(pinsBank0[s], LOW); 
        digitalWrite(pinsBank0[foundIndex], LOW); // set the pins direkt if you push faster a Off-button than its released (remember, its a 5s "deleay" after an on pin is set to off again
        longOffCleanUpTimeBank0[s] = 0; // remove off timestamp
        longOnCleanUpTimeBank0[foundIndex] = 0; // remove on timestamp - same issue as with digital write above...
      }
    }
}

void WatchdogBank1()
{
    for(byte s = 0; s < 16; s++)
    {
      if(longOnCleanUpTimeBank1[s] > 0 && longOnCleanUpTimeBank1[s]<= now())
      {
        stayOnPinsBank1[s] = 1;  
        digitalWrite(pinsBank1[s], LOW); 
        longOnCleanUpTimeBank1[s] = 0; 
      }
      if(longOffCleanUpTimeBank1[s] > 0 && longOffCleanUpTimeBank1[s]<= now())
      {
        byte foundIndex = indexOfOffPinPairBank1[s];
        stayOnPinsBank1[foundIndex] = 0;
        digitalWrite(pinsBank1[s], LOW);
        digitalWrite(pinsBank1[foundIndex], LOW);
        longOffCleanUpTimeBank1[s] = 0;
        longOnCleanUpTimeBank1[foundIndex] = 0;
      }
    }
}

void WatchdogBank2()
{
    for(byte s = 0; s < 16; s++)
    {
      if(longOnCleanUpTimeBank2[s] > 0 && longOnCleanUpTimeBank2[s]<= now())
      {
        stayOnPinsBank2[s] = 1;  
        digitalWrite(pinsBank2[s], LOW); 
        longOnCleanUpTimeBank2[s] = 0; 
      }
      if(longOffCleanUpTimeBank2[s] > 0 && longOffCleanUpTimeBank2[s]<= now())
      {
        byte foundIndex = indexOfOffPinPairBank2[s];
        stayOnPinsBank2[foundIndex] = 0;
        digitalWrite(pinsBank2[s], LOW);
        digitalWrite(pinsBank2[foundIndex], LOW);
        longOffCleanUpTimeBank2[s] = 0;
        longOnCleanUpTimeBank2[foundIndex] = 0;
      }
    }
}

void WatchdogBank3()
{
	for (byte s = 0; s < 16; s++)
	{
		if (longOnCleanUpTimeBank3[s] > 0 && longOnCleanUpTimeBank3[s] <= now())
		{
			stayOnPinsBank3[s] = 1;
			digitalWrite(pinsBank3[s], LOW);
			longOnCleanUpTimeBank3[s] = 0;
		}
		if (longOffCleanUpTimeBank3[s] > 0 && longOffCleanUpTimeBank3[s] <= now())
		{
			byte foundIndex = indexOfOffPinPairBank3[s];
			stayOnPinsBank3[foundIndex] = 0;
			digitalWrite(pinsBank3[s], LOW);
			digitalWrite(pinsBank3[foundIndex], LOW);
			longOffCleanUpTimeBank3[s] = 0;
			longOnCleanUpTimeBank3[foundIndex] = 0;
		}
	}
}


/**********************************************************************************************************************************/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++ HELPER METHODS ++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/**********************************************************************************************************************************/

String getStringPartByNr(String data, char separator, int index)
{
  int stringData = 0; 
  String dataPart = "";    
  for(int i = 0; i<data.length(); i++) 
  {      
    if(data[i]==separator) 
      stringData++;
    else if(stringData==index) 
      dataPart.concat(data[i]);
    else if(stringData>index) 
    {
      return dataPart;
      break;  
    }
  }
  return dataPart;
}

char * int2bin(unsigned int x)
{
  static char mybuffer[17];
  for (int i=0; i<16; i++) mybuffer[15-i] = '0' + ((x & (1 << i)) > 0);
  mybuffer[16] ='\0';
  
  return mybuffer;
}

unsigned int bin2dec(String bin)
{
  unsigned int i = 1;
  unsigned int sum = 0;
 
  for (int m = 15; m >= 0; m--)  // do it from the back...
  {
    sum += (bin.charAt(m)-48) * i;
    i = i * 2;
  }     
  
  return sum;
}

String revertBinaryString(String aString)
{
  String temp;
  for (int a = 15; a >= 0; a--)
  {    
    temp += aString.charAt(a);
  }
  return temp;
}

void readSDSettings()
{
 char character;
 String settingName;
 String settingValue;
 myFile = SD.open("config.cfg");
 if (myFile) 
 {
	while (myFile.available()) 
	{
		character = myFile.read();
		while((myFile.available()) && (character != '['))
		{
			character = myFile.read();
		}
		character = myFile.read();
		while((myFile.available()) && (character != '='))
		{
			settingName = settingName + character;
			character = myFile.read();
		}
		character = myFile.read();
		while((myFile.available()) && (character != ']'))
		{
			settingValue = settingValue + character;
			character = myFile.read();
		}
		if(character == ']')
		{ 
	   
	//   //Debuuging Printing
//			Serial.print("Name:");
//			Serial.println(settingName);
//			Serial.print("Value :");
//			Serial.println(settingValue);
			
			if(settingName == "ajaxUrl")
				ajaxUrl = settingValue;
		
			else if(settingName == "deviceIp")
				deviceIp = settingValue;

			else if (settingName == "gatewayIp")
				gatewayIp = settingValue;

			else if (settingName == "title")
				title = settingValue;

			else if (settingName == "jsUrl")
				jsUrl = settingValue;

			else if (settingName == "cssUrl")
				cssUrl = settingValue;

			else if (settingName == "faviconUrl")
				faviconUrl = settingValue;

			else if (settingName == "dotUrl")
				dotUrl = settingValue;

			else if (settingName == "jqueryUrl")
				jqueryUrl = settingValue;

			else if (settingName == "strpinsBank0")
				strpinsBank0 = settingValue;
			else if (settingName == "strpinsBank1")
				strpinsBank1 = settingValue;
			else if (settingName == "strpinsBank2")
				strpinsBank2 = settingValue;
			else if (settingName == "strpinsBank3")
				strpinsBank3 = settingValue;
			else if (settingName == "stris5sPinBank0")
				stris5sPinBank0 = settingValue;
			else if (settingName == "stris5sPinBank1")
				stris5sPinBank1 = settingValue;
			else if (settingName == "stris5sPinBank2")
				stris5sPinBank2 = settingValue;
			else if (settingName == "stris5sPinBank3")
				stris5sPinBank3 = settingValue;
			else if (settingName == "strisOffPinBank0")
				strisOffPinBank0 = settingValue;
			else if (settingName == "strisOffPinBank1")
				strisOffPinBank1 = settingValue;
			else if (settingName == "strisOffPinBank2")
				strisOffPinBank2 = settingValue;
			else if (settingName == "strisOffPinBank3")
				strisOffPinBank3 = settingValue;
			else if (settingName == "strindexOfOffPinPairBank0")
				strindexOfOffPinPairBank0 = settingValue;
			else if (settingName == "strindexOfOffPinPairBank1")
				strindexOfOffPinPairBank1 = settingValue;
			else if (settingName == "strindexOfOffPinPairBank2")
				strindexOfOffPinPairBank2 = settingValue;
			else if (settingName == "strindexOfOffPinPairBank3")
				strindexOfOffPinPairBank3 = settingValue;
			else if (settingName == "strpushDurationBank0")
				strpushDurationBank0 = settingValue;
			else if (settingName == "strpushDurationBank1")
				strpushDurationBank1 = settingValue;
			else if (settingName == "strpushDurationBank2")
				strpushDurationBank2 = settingValue;
			else if (settingName == "strpushDurationBank3")
				strpushDurationBank3 = settingValue;

			settingName = "";
			settingValue = "";
		}
	}
	myFile.close();
 } 
 else 
 {
 // if the file didn't open, print an error:
	//Serial.println("error opening settings.txt");
 }
 }


void getByteArray(String strResponse, byte tempByteArray[])
{
	String tempBy[16];
	getStringArray(strResponse, tempBy);

	for (int a = 0; a < 16; a++)
		tempByteArray[a] = (byte)tempBy[a].toInt();
}

void getBoolArray(String strResponse, boolean tempBoolArray[])
{
	String tempBol[16];
	getStringArray(strResponse, tempBol);

	for (int a = 0; a < 16; a++)
		tempBoolArray[a] = (boolean)tempBol[a].toInt();
}


void getStringArray(String strResponse, String tempResult[])
{
	// Convert from String Object to String.
	char buf[strResponse.length() + 1];
	strResponse.toCharArray(buf, sizeof(buf));
	char *p = buf;
	char *str;
	int cntr;
	while ((str = strtok_r(p, ",", &p)) != NULL) // delimiter is the semicolon
	{
		tempResult[cntr] = String(str);
		cntr += 1;
	}
}



/**************************************************************************************************************************************************/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++ INDEX AND PAGE CONTROL METHODS ++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/**************************************************************************************************************************************************/

void setPinsOfBank(byte bankNr, String aRevertedBinaryString)
{
  byte result[16];
  byte lengthOfBinary = aRevertedBinaryString.length();

 /* 
  Serial.print(bankNr);
  Serial.print("# RevertedString: ");
  Serial.println(aRevertedBinaryString);
*/
  for(byte b = 0; b < lengthOfBinary; b++)
  {
    result[b] = aRevertedBinaryString.charAt(b); // get a char out of a sting at position. remember: the value is the char value not 0 or 1... -48!
                               
    byte value = result[b]-48;

/*
	Serial.print(" Stelle ");
	Serial.print(b);
	Serial.print(" Value");
	Serial.print(value);
*/

	if (value == 1) // if its 1 => turn pin on!
    {
      if(bankNr ==0)
      {
        if(is5sPinBank0[b] && !stayOnPinsBank0[b]) // set the timestamp when status should be changed only if its a long ON -button and its not allready set :P
          longOnCleanUpTimeBank0[b] = now()+pushDurationBank0[b]; 
        if(isOffPinBank0[b])
          longOffCleanUpTimeBank0[b] = now()+pushDurationBank0[b];
		if (!stayOnPinsBank0[b]) // if its not set as a already-on-button, than call the arduino pin. if its set to "already on", do nothing, it might be off meanwhile...
		{
			digitalWrite(pinsBank0[b], HIGH);
		}
      }
      if(bankNr ==1)
      {
        if(is5sPinBank1[b] && !stayOnPinsBank1[b])
          longOnCleanUpTimeBank1[b] = now()+pushDurationBank1[b];
        if(isOffPinBank1[b])
          longOffCleanUpTimeBank1[b] = now()+pushDurationBank1[b];
        if(!stayOnPinsBank1[b])
          digitalWrite(pinsBank1[b], HIGH);
        //Serial.println(pinsBank1[b]);
      }
      if(bankNr ==2)
      {
        if(is5sPinBank2[b] && !stayOnPinsBank2[b])
          longOnCleanUpTimeBank2[b] = now()+pushDurationBank2[b];
        if(isOffPinBank2[b])
          longOffCleanUpTimeBank2[b] = now()+pushDurationBank2[b];
        if(!stayOnPinsBank2[b])
          digitalWrite(pinsBank2[b], HIGH);
      }  
	  if (bankNr == 3)
	  {
		  if (is5sPinBank3[b] && !stayOnPinsBank3[b])
			  longOnCleanUpTimeBank3[b] = now() + pushDurationBank3[b];
		  if (isOffPinBank3[b])
			  longOffCleanUpTimeBank3[b] = now() + pushDurationBank3[b];
		  if (!stayOnPinsBank3[b])
			  digitalWrite(pinsBank3[b], HIGH);
	  }
	}
    else
    {
		if (bankNr == 0)
		{
			digitalWrite(pinsBank0[b], LOW);
		}
      if(bankNr ==1)
        digitalWrite(pinsBank1[b], LOW);
      if(bankNr ==2)
        digitalWrite(pinsBank2[b], LOW);
	  if (bankNr == 3)
		  digitalWrite(pinsBank3[b], LOW);
	}
  }
}

int getPinsOfBank(byte bankNr)
{
  String temp1;
  for(int x = 15; x >= 0; x--)
  {
    if(bankNr ==0)
    {
      if(stayOnPinsBank0[x] == 1) // if its a already-on button, check this. if its on, dont call the arduino, because the digital status is off already.. just show him as on...
        temp1 += '1';
      else
        temp1 += digitalRead(pinsBank0[x]);      
    }
    if(bankNr ==1)
      if(stayOnPinsBank1[x] == 1)
        temp1 += '1';
      else
        temp1 += digitalRead(pinsBank1[x]);      
    if(bankNr ==2)
      if(stayOnPinsBank2[x] == 1)
        temp1 += '1';
      else
        temp1 += digitalRead(pinsBank2[x]);      
	if (bankNr == 3)
		if (stayOnPinsBank3[x] == 1)
			temp1 += '1';
		else
			temp1 += digitalRead(pinsBank3[x]);
  }
  return bin2dec(temp1);
}

String getComplete()
{
  String temp;
  for (int a = 0; a < 4; a++)
  {
    unsigned int tempval = getPinsOfBank(a); 
    if(a <3)
      temp += String(tempval)+ "|";
    else
      temp += tempval;      
  }
  return temp;
}

void initBanksFromSDCarsd()
{
	getByteArray(strpinsBank0, pinsBank0);
	getByteArray(strpinsBank1, pinsBank1);
	getByteArray(strpinsBank2, pinsBank2);
	getByteArray(strpinsBank3, pinsBank3);

	getBoolArray(stris5sPinBank0, is5sPinBank0);
	getBoolArray(stris5sPinBank1, is5sPinBank1);
	getBoolArray(stris5sPinBank2, is5sPinBank2);
	getBoolArray(stris5sPinBank3, is5sPinBank3);
	getBoolArray(strisOffPinBank0, isOffPinBank0);
	getBoolArray(strisOffPinBank1, isOffPinBank1);
	getBoolArray(strisOffPinBank2, isOffPinBank2);
	getBoolArray(strisOffPinBank3, isOffPinBank3);

	getByteArray(strindexOfOffPinPairBank0, indexOfOffPinPairBank0);
	getByteArray(strindexOfOffPinPairBank1, indexOfOffPinPairBank1);
	getByteArray(strindexOfOffPinPairBank2, indexOfOffPinPairBank2);
	getByteArray(strindexOfOffPinPairBank3, indexOfOffPinPairBank3);
	getByteArray(strpushDurationBank0, pushDurationBank0);
	getByteArray(strpushDurationBank1, pushDurationBank1);
	getByteArray(strpushDurationBank2, pushDurationBank2);
	getByteArray(strpushDurationBank3, pushDurationBank3);
}


/*********************************************************************************************************************************/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++ THE WEB PAGES ++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/*********************************************************************************************************************************/

void GetPage(EthernetClient client, byte bankNr)
{
  unsigned int mu = getPinsOfBank(bankNr);
  client.println("HTTP/1.1 200 OK"); //send new page
  client.println("Content-Type: text/html");
  client.println("Access-Control-Allow-Origin: *");
  client.println();
  client.print("myCB({'v':");
  client.print(mu);
  client.print("})");
}

void GetAllPage(EthernetClient client)
{
  String mu = getComplete();
  client.println("HTTP/1.1 200 OK"); //send new page
  client.println("Content-Type: text/html");
  client.println("Access-Control-Allow-Origin: *");
  client.println();
  client.print("xx({\"v\": \"");
  client.print(mu);
  client.print("\"})");
}


void SetPage(EthernetClient client, byte bankNr)
{
  client.println("HTTP/1.1 200 OK"); //send new page
  client.println("Content-Type: text/html");  
}

void MainPage(EthernetClient client, boolean isLocal)
{
   client.println("HTTP/1.1 200 OK"); //send new page
   client.println("Content-Type: text/html");
   client.println();
   client.println("<!DOCTYPE html>");     
   client.println("<HTML>");
   client.println("<HEAD>");
   client.println("<meta http-equiv=\"Cache-control\" content=\"no-cache\"><meta http-equiv=\"Expires\" content=\"0\">");
   client.print("<script type=\"text/javascript\" src=\"http://code.jquery.com/jquery-1.11.2.min.js\"></script>");
   client.print("<script type=\"text/javascript\" src=\"");
   client.print(jqueryUrl);
   client.println("\"></script>");
   client.print("<script>var configAddress='");
   if(isLocal)
     client.print(deviceIp);
   else
     client.print(ajaxUrl);     
   client.println("';</script>");
   client.print("<script src=\"");
   client.print(jsUrl);
   client.println("\"></script>");
   client.print("<link rel=\"Stylesheet\" href=\"");
   client.print(cssUrl);
   client.println("\" type=\"text/css\">");
   client.print("<link rel=\"shortcut icon\" href=\"");
   client.print(faviconUrl);
   client.println("\">");
   client.print("<TITLE>");
   client.print(title);
   client.println("</TITLE>");
   client.println("</HEAD>");
   client.println("<BODY>");
   client.print("<div id=\"container\"><div class=\"rotate\" id=\"bb\">Bank 0 <img id=\"greenBullet0\" src=\"");
   client.print(dotUrl);
   client.println("\"></div><div id=\"bank0\"></div></div>");
   client.print("<div id=\"container\"><div class=\"rotate\">Bank 1 <img id=\"greenBullet1\" src=\"");
   client.print(dotUrl);
   client.println("\"></div><div id=\"bank1\"></div></div>");
   client.print("<div id=\"container\"><div class=\"rotate\">Bank 2 <img id=\"greenBullet2\" src=\"");
   client.print(dotUrl);
   client.println("\"></div><div id=\"bank2\"></div></div>");
   client.print("<div id=\"container\" class=\"myLast\"><div class=\"rotate\">Bank 3 <img id=\"greenBullet3\" src=\"");
   client.print(dotUrl);
   client.println("\"></div><div id=\"bank3\"></div></div><div id=\"tempResponse\"></div>");
   client.println("</BODY>");
   client.println("</HTML>");
}



