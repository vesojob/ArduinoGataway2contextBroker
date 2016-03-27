#include <ArduinoJson.h>
//Servo control
#include <Servo.h>
Servo servoMain; // Define our Servo gataway

#include <SPI.h>
#include <SoftwareSerial.h>

/*//memory check
#include <MemoryFree.h>
#include <pgmStrToRAM.h>
*/

//Ethernet shield
#include <Ethernet.h>

//Light meter
#include "TSL2561.h"
#include <Wire.h>
#include <RTClib.h>

//RFID
#include <MFRC522.h>

//LSD display
#include <LiquidCrystal.h>

//==============================================================================================  
//
//  MACROS
//
//==============================================================================================  

//Messages 
#define msgInit "Init.."
#define msgSend "Send.."
#define msgConnect "Conn.."
#define msgDisconnect "Disconn.."
#define msgErr "Fail"
#define msgReady "Ready"

//SENSOR ID - Arduino
#define SID       1

//ContextBroker
#define url             "192.168.0.26"  
#define PORT            1026 



//5 Seconds wait before refresh
#define LCD_WAIT_TIME_REFRESH 5

//RFID Reader
#define RST_PIN    9   //change defaults for RFID 
#define SS_PIN    8 //change defaults for RFID

#define TEMP_PIN   0 //temo pin
//==============================================================================================  
//
//  Global Variables
//
//==============================================================================================  

//SDA BUS==========================

//TIME
RTC_DS1307 RTC;

// LUX sensor Use TSL2561_ADDR_LOW (0x29) or TSL2561_ADDR_HIGH (0x49) respectively
TSL2561 tsl(TSL2561_ADDR_FLOAT);

//END SDA BUS==========================

//TMP36 Pin Variables
int sensorPin = TEMP_PIN;

//Create MFRC522 instance
MFRC522 mfrc522(SS_PIN, RST_PIN);

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(7, 6, 5, 4, 3, 2);

//Ethernet config
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

//Client
EthernetClient client;

//Global serial Tag
unsigned long serialTag = 0;

unsigned long nLCDLastTimeRefresh= 1;

//==============================================================================================  
//
// Namespaces
//
//==============================================================================================  

/*namespace CHTTPRequest {
  typedef struct _AttrData
  {
      String sAttrName;
      String sAttrType;
      String sAttrValue;
  } CAttrData ;
 
//----------------------------------------------------------------------------------------------    
//
//----------------------------------------------------------------------------------------------  
int sendContext(const char * szTypeID
                  , unsigned long nElementID
                  ,  CAttrData   arrAttributes[]
                  ,  int nArrElemCount
                  , unsigned long timestamp )
  {
    
          lcd.setCursor(0, 1);
          lcd.print(nElementID);
          if (client.connect(url, PORT)) 
          {
            long conLength = 0;
            
            String arrCBStrings[nArrElemCount + 3];
            int nStrIndex = 0;
            
            arrCBStrings[nStrIndex] = "{\"contextElements\":[{\"type\":\"";
            arrCBStrings[nStrIndex]+=  szTypeID;
            arrCBStrings[nStrIndex]+=  "\",\"isPattern\":\"false\",\"id\":\"";
            arrCBStrings[nStrIndex]+=  nElementID;
            
            nStrIndex++;
            
            arrCBStrings[nStrIndex]= "\",\"attributes\":[{\"name\":\"ts\",\"type\":\"string\",\"value\":\"";
            arrCBStrings[nStrIndex] += timestamp;
   
            nStrIndex++;
    
            for (int nCurrAttrIndex = 0; nCurrAttrIndex < nArrElemCount; nCurrAttrIndex++)
            {
              arrCBStrings[nStrIndex]= "\"},{\"name\":\"";
              arrCBStrings[nStrIndex]+= arrAttributes[nCurrAttrIndex].sAttrName;
              arrCBStrings[nStrIndex]+= "\",\"type\":\"";
              arrCBStrings[nStrIndex]+= arrAttributes[nCurrAttrIndex].sAttrType;
              arrCBStrings[nStrIndex]+= "\",\"value\":\"";
              arrCBStrings[nStrIndex]+= arrAttributes[nCurrAttrIndex].sAttrValue;
    
              nStrIndex++;  
            }
            
            nStrIndex++;
            arrCBStrings[nStrIndex] = "\"}]}],\"updateAction\":\"APPEND\"}";
            
            //Check Context Length: 
            
            for (int nCurrLenIndex = 0; nCurrLenIndex <= nStrIndex; nCurrLenIndex++)
            {
            //  Serial.println(arrCBStrings[nCurrLenIndex]);
              conLength+= arrCBStrings[nCurrLenIndex].length();
            }

            Serial.println("connected");
            Serial.println(conLength);
            
            // Make a HTTP request:
            client.println("POST  /ngsi10/updateContext");
            //client.println("Connection: keep-alive");
            client.println("Accept: application/json");
            client.println("Content-Type: application/json");
            //client.println("Content-Type:   text/plain; charset=UTF-8");
            //client.print("X-Auth-Token: "); 
            //client.println("VMfS1db5lyV3mtJ7YSqAqPFHR9cu9m");
          
            //client.println("Connection: close");
            client.print("Content-Length: ");
            client.println(conLength);
            client.println(); 
            for (int nCurrStrIndex = 0; nCurrStrIndex <= nStrIndex; nCurrStrIndex++)
            {
              client.print(arrCBStrings[nCurrStrIndex]);
            }
            client.println();
            //client.println("Connection: close");
            //Serial.println(data);
            client.stop();
            //delay (1000);
            return 0;
         }
         else 
         {
          // if you didn't get a connection to the server:
             Serial.println(msgErr);
                  lcd.setCursor(10, 0);
                  lcd.print(msgErr);
         }  
     }
  
//----------------------------------------------------------------------------------------------    
//
};
*/
//----------------------------------------------------------------------------------------------  
//
//
//----------------------------------------------------------------------------------------------  

void setup(){
    //Init serial bus
    Serial.begin(9600);
    
    //Init SPI bus
    SPI.begin();      
    
    //Init Time
    Wire.begin();
    RTC.begin();
     
    //Init LCD
    lcd.begin(16, 2);
    
    //Init MFRC522
    mfrc522.PCD_Init();   

    //Wellcome message
    Serial.println(msgInit);  
    lcd.print(msgInit);
    
    //Init Ethernet connection:
    if (Ethernet.begin(mac) == 0){
        Serial.println(msgErr);
        lcd.clear();
        lcd.print(msgErr);
        delay(10000);
    }
    delay(1000);  //give the Ethernet shield a second to initialize:
    client.setTimeout(500);
    lcd.clear();
      
    Serial.println(Ethernet.localIP());
    lcd.print(Ethernet.localIP());
    
    //Init Light module    
    tsl.begin();
    //tsl.setGain(TSL2561_GAIN_0X);         // set no gain (for bright situtations)
    tsl.setGain(TSL2561_GAIN_16X);      // set 16x gain (for dim situations)
    // longer timelines are slower, but are good in very low light situtations!
    tsl.setTiming(TSL2561_INTEGRATIONTIME_13MS);  // shortest integration time (bright light)
     
    lcd.setCursor(0, 1);
    Serial.println(msgReady); 
    lcd.print(msgReady);

    //LED TEST
    //delay(2000); 
    pinMode(A2, OUTPUT);
    //digitalWrite(A2, HIGH);
    //delay (1000);
    //digitalWrite(A2, LOW);
    

    //SERVO TEST
    pinMode(A1, OUTPUT);
    servoMain.attach(A1);
   
   // servoMain.write(0);
   // delay (2000);
    //servoMain.write(90);
    //delay (2000);
    //servoMain.write(170);
    //delay (2000);
}

//----------------------------------------------------------------------------------------------  
//  This function will get context Value, it's take only 200 simbols from context massage. If it take more them buffer overflow
//
//---------------------------------------------------------------------------------------------- 

int getContext(unsigned long tagElementID){
    
   // We declare a client with wifi connection
    Serial.println(msgSend);
   // If I connect with the server in the port (1026) report via serial
   if (client.connect(url, PORT)) {
    //delay (1000);
    Serial.println(msgConnect);
    // Make a HTTP request:
    client.print(F("GET /v1/contextEntities/"));
    client.print(tagElementID);
    client.println(F("/attributes/divert HTTP/1.1"));
    client.println(F("Accept: application/json"));
    client.println(F("Content-Type: application/json"));
   // client.print("X-Auth-Token: ");    
   // client.println("59CO7vvjVhv7_06oJTmTinV5t2A73q0IDLdcaib3_K7hRk6rF2lli-DZUKIljnU9T2mIY9DtwGxFKeuQYqVtGg");
    client.println(F("Connection: close"));
    client.println();
 
   }else {
    // if you didn't get a connection to the server:
    Serial.println(msgErr);
 // return 0;
  }
  // if the server's disconnected, stop the client:
  if (!client.connected()) {
    Serial.println();
    Serial.println(msgDisconnect);
    client.stop();
    client.flush();
    //delay (10000);
   
   // do nothing forevermore:
   // while(true);
   return -1;
  }
  // Read and print bytes comming from the server if available
  String buffer = "";
int var =0;   
bool begin = false;
while (client.available() || !begin ) {
    //max string overflow prevention - 250 
    var++;
    //Serial.println(var);
    char in = client.read();

    if (in == '{') {
        begin = true;
    }

    if (begin) buffer += (in);

    if (in == '}' || var == 250) {
        client.stop();
        client.flush();
        break;
    }

    delay(1);
}
  

   // Serial.println("BUFFER");
    Serial.println(buffer);
 
    buffer.replace("\n","");
    buffer.replace("\t","");

  int startIndex = buffer.indexOf("{");
  int endIndex = buffer.length();

  Serial.println(buffer);
  String json = buffer.substring(startIndex, endIndex);
  Serial.println(json);

  int val = json.indexOf("\"value\" : \"1\"");
   buffer = "";   
   //Serial.println("VAL=");
   Serial.print(val);
  
  if(val == -1){
      //Orion responds that the LED is off
      digitalWrite(A2, LOW);
      servoMain.write(0);
    }
  else{
    digitalWrite(A2, HIGH);
    servoMain.write(90);
  }
    
  return 0;
}
 

//----------------------------------------------------------------------------------------------  
// Send Context to ContextBroker, all of the string is in the flash memory on the chip and it is replased by their simbol lengh.
// This is reduse buffert overflow of the string value.
//----------------------------------------------------------------------------------------------  

int sendContext( unsigned long tag, unsigned long timestamp, unsigned long lux, String temp  ){
        lcd.setCursor(0, 1);
        lcd.print(tag);

     //    Serial.println("TEST");
if (client.connect(url, PORT)) {
      long conLength = 0;
       String data;
       String data1;
       String data2;
       String data3;
      conLength = 57;
      Serial.println(conLength);
      data= "\"" ;
      data = data + tag;
      
      conLength = conLength + data.length();
      Serial.println(data);
      Serial.println(conLength);
      
      conLength = conLength +  53;
      
      Serial.println(conLength);
      data1= "\"" ;
      data1 = data1 + timestamp;
      
      conLength = conLength + data1.length();
      Serial.println(conLength);
      
      conLength = conLength +  45;

      data2= "\"" ;
      data2 = data2 + lux;
      
      Serial.println(data2);
      conLength = conLength + data2.length();
      
      conLength = conLength +  38;

       data3= "\"" ;
      data3 = data3 + temp;
      conLength = conLength + data3.length();
      
      Serial.println(conLength);
      conLength = conLength +  30;

    
      Serial.println(conLength);
      
      Serial.println("connected");
      
      
      
      // Make a HTTP request:
      client.println(F("POST  /ngsi10/updateContext"));
      //client.println("Connection: keep-alive");
      client.println(F("Accept: application/json"));
      client.println(F("Content-Type: application/json"));
      //client.println("Content-Type:   text/plain; charset=UTF-8");
      //client.print("X-Auth-Token: "); 
      //client.println("VMfS1db5lyV3mtJ7YSqAqPFHR9cu9m");
    
      //client.println("Connection: close");
      client.print(F("Content-Length: "));
      client.println(conLength);
      client.println(); 
      client.println(F("{\"contextElements\":[{\"type\":\"b\",\"isPattern\":\"false\",\"id\":"));
      client.print(data);
      client.print(F("\",\"attributes\":[{\"name\":\"ts\",\"type\":\"string\",\"value\":"));
      client.print(data1);
      client.print(F("\"},{\"name\":\"divert\",\"type\":\"string\",\"value\":"));
      client.print(data2);
      client.print(F("\"},{\"name\":\"lx\",\"type\":\"int\",\"value\":"));
      client.print(data3);
      client.print(F("\"}]}],\"updateAction\":\"APPEND\"}"));
      client.println();
      client.println("Connection: close");
      //Serial.println(data);
      client.stop();
      //delay (1000);
      return 0;
   }
    else {
    // if you didn't get a connection to the server:
       Serial.println(msgErr);
            lcd.setCursor(10, 0);
            lcd.print(msgErr);
   }  
}

//----------------------------------------------------------------------------------------------  
//
//
//----------------------------------------------------------------------------------------------  
void loop() {
 Serial.println("Loop");
//=====  

  lcd.clear();
  DateTime now = RTC.now();
  /* Serial.print("Timestamp= ");
   Serial.println(now.unixtime());
    Serial.print(now.year());
    Serial.print('/');
    Serial.print(now.month());
    Serial.print('/');
    Serial.print(now.day());
    Serial.print(' ');
    Serial.print(now.hour());
    Serial.print(':');
    Serial.print(now.minute());
    Serial.print(':');
    Serial.println(now.second());
  */  
   
    lcd.setCursor(0, 1);
    lcd.print(now.hour());
    lcd.print(":");
    lcd.print(now.minute());
    lcd.print(":");
    lcd.print(now.second());
  
  //temperature
  int reading = analogRead(sensorPin);  
  float voltage = reading * 5.0;
  voltage /= 1024.0; 
  // Serial.print(voltage); 
  //Serial.println(" V");
  float temperatureC = (voltage - 0.5) * 100 ;  
  //Serial.print(temperatureC); 
   
  char str_temp[6];
   /* 4 is mininum width, 2 is precision; float value is copied onto str_temp*/
     dtostrf(temperatureC, 4, 2, str_temp);
     //sprintf(temperature,"%s F", str_temp);

    Serial.println(str_temp);
    Serial.println(" C");

    // This can take 13-402 milliseconds! Uncomment whichever of the following you want to read
    uint16_t x = tsl.getLuminosity(TSL2561_VISIBLE);     
    uint32_t lum = tsl.getFullLuminosity();
    uint16_t ir, full;
    ir = lum >> 16;
    full = lum & 0xFFFF;
    //Serial.print("Visible: "); Serial.print(full - ir);   Serial.print("\t");
    //Serial.print("Lux: "); Serial.println(tsl.calculateLux(full, ir));
    lcd.setCursor(0, 0);
    lcd.print("T=");
    // lcd.setCursor(2, 0);
    lcd.print(temperatureC);
    lcd.setCursor(9, 0);
    lcd.print("L=");
    //lcd.setCursor(11, 0);
    lcd.print(full - ir);

 
        
        //RFID card operation     
        int checkOrion;     
        int progress;
        delay(1000);
        // Look for new cards
        if ( ! mfrc522.PICC_IsNewCardPresent()) {
                return;
        } 
        // Select one of the cards
        if ( ! mfrc522.PICC_ReadCardSerial()){ 
          return;  
          } 
        
        dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);
        
        //Serial.println (serialTag);
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Sending..");

//      Serial.print("  freeMemory()=");
//      Serial.println(freeMemory());
        
                                   
        // Declare a string to store the request buffer that arduino is listening to
        //Execute the call to the server
       
        checkOrion = getContext(serialTag);  
        
        //Genereting Card serial niumber
        progress = sendContext(serialTag , now.unixtime(), 2 , str_temp);   
          //Serial.println(progress);
          
         
          if(!progress){
            Serial.println(msgReady);
            lcd.setCursor(10, 0);
            lcd.print(msgReady);
            
          delay(1000);
          
            }
          
  delay(2000);
}

//--------------------------------------------------------------------------------------------------------------------------

void dump_byte_array(byte *buffer, byte bufferSize) {
  // String sID;
        if (mfrc522.uid.size <= 4)
        {
          serialTag = 0;
            for (int nCurrByte = 0; nCurrByte < mfrc522.uid.size; nCurrByte++ )
              {
                  unsigned long nTempVal = mfrc522.uid.uidByte[nCurrByte]; //left bytes side read
                  //unsigned long nTempVal = mfrc522.uid.uidByte[((mfrc522.uid.size-1) -   nCurrByte)]; //right bytes side read
              
                  nTempVal<<= (((mfrc522.uid.size-1) -   nCurrByte) * 8);
                  serialTag += nTempVal;
              }
           Serial.println(serialTag);
        }
        else
        {
         // Serial.println("Error! source buffer is larger than 4 bytes!");
        }
}

//------------------------------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------------------

