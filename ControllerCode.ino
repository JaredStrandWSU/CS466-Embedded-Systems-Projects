/*
 *  HARDWARE HACKATHON 2018 - Team ASIC
 *  This sketch demonstrates how to scan a WiFi network, return your MAC address, then update a state machine based on server response. 
 *  The API is written in python and flask
 *  The web interface is written in Bootstrap, JavaScript, jQuery, and CSS
 *  Board Used: ESP8266 microcontroller NodeMCU ESP-12E WiFimicro-controller
 */

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>

ESP8266WiFiMulti WiFiMulti;
WiFiClientSecure client;
HTTPClient http;

const char* ssid = "hackathon-sln353";
const char* password = "";

const int REED_SWITCH = 9; // input
const int SPEAKER =    2; // output
const int LED_YELLOW = 5;  // output
const int LED_RED    = 14;  // output
const int LED_GREEN  = 4;  // output

const int c = 261;
const int d = 294;
const int e = 329;
const int f = 349;
const int g = 391;
const int gS = 415;
const int a = 440;
const int aS = 455;
const int b = 466;
const int cH = 523;
const int cSH = 554;
const int dH = 587;
const int dSH = 622;
const int eH = 659;
const int fH = 698;
const int fSH = 740;
const int gH = 784;
const int gSH = 830;
const int aH = 880;

int counter = 0;

void setup() {
  pinMode(REED_SWITCH, INPUT_PULLUP);
  pinMode(SPEAKER,    OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_RED,    OUTPUT);
  pinMode(LED_GREEN,  OUTPUT);
  
  Serial.begin(57600);
  delay(10);

  // We start by connecting to a WiFi network
  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP("hackathon-sln353", "");

  Serial.println();
  Serial.println();
  Serial.print("Wait for WiFi... ");

  while (WiFiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  delay(500);

  Serial.println("Setup done");
}
 
void loop() {
  //Variables
  int prevState = -1;
  int state = 4;
  int reedSwitchInput = 0;
  int httpCode = 0;
  char sReedSwitchInput[5];
  byte mac[6];

  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_GREEN, HIGH);
  digitalWrite(LED_YELLOW, LOW);
  
  Serial.println("Begin registration with Server");
  
  //Make request to register with server "I'm alive"
    WiFi.macAddress(mac);

    Serial.print("MAC: ");
    Serial.print(mac[5],HEX);
    Serial.print(":");
    Serial.print(mac[4],HEX);
    Serial.print(":");
    Serial.print(mac[3],HEX);
    Serial.print(":");
    Serial.print(mac[2],HEX);
    Serial.print(":");
    Serial.print(mac[1],HEX);
    Serial.print(":");
    Serial.println(mac[0],HEX);

    String macString = "12345";
    http.begin("http://team-asic.czhanacek.com/sensors/register");
    
    do
    {
      httpCode = http.POST("mac=" + macString);

      if(httpCode == 500)
      {
        Serial.println("500: Internal Server Error during Registration"); 
      }
      else if(httpCode == 200)
      {
        Serial.println("200: Successful Registration"); 
      }
      else
      {
        Serial.println("0: Request Error"); 
      }
      delay(1500); 
    } while (httpCode == 500 || httpCode == 0);
    
    http.end();

  //Run Microcontroller program while(1) or while(connectedToHost)
  Serial.println("Entering state control loop...");
  while(1)
  {
    //Get the state from server

      //Read the value of the reed switch
      reedSwitchInput = digitalRead(REED_SWITCH);
      //Serial.print("Reed Switch state: ");
      //Serial.print(reedSwitchInput);
      //Serial.println("");
      
      //int to string
      itoa(reedSwitchInput, sReedSwitchInput, 10);
      Serial.println("Reed Switch Input converted Successfully");
      Serial.println(sReedSwitchInput);
      
      //Send the new reed value and any other current variables or parameters to server, server responds with state value
      http.begin("http://team-asic.czhanacek.com/sensors/report");
      state = http.POST("http://team-asic.czhanacek.com/sensors/report?mac=" + macString + "&val=" + sReedSwitchInput);
      Serial.println("state request successfully received");
      Serial.println(state);
    
    //switch statement based on state value
    switch (state) {
    case 201: //Armed
      // statements
      Serial.println("CASE 1 ARMED");
      if (prevState == state)
      {
        //Do Nothing
      }
      else
      {
        digitalWrite(LED_RED, HIGH);
        digitalWrite(LED_GREEN, LOW);
        digitalWrite(LED_YELLOW, LOW);
      }
      break;
    case 202: //Disarmed
      // statements
      Serial.println("CASE 2 DISARMED");
      if (prevState == state)
      {
        //Do Nothing
      }
      else
      {
        digitalWrite(LED_RED, LOW);
        digitalWrite(LED_GREEN, HIGH);
        digitalWrite(LED_YELLOW, LOW);
      }
      break;
    case 203: //Active (triggered)
      // statements
      Serial.println("CASE 3 TRIGGERED");
      if (prevState == state)
      {
        beep(fSH, 200);
        beepLoud();
      }
      else
      {
        digitalWrite(LED_RED, HIGH);
        digitalWrite(LED_GREEN, LOW);
        digitalWrite(LED_YELLOW, LOW);
        beep(fSH, 200);
        beepLoud();
      }
      break;
    case 204: //Default state
      // statements
      Serial.println("CASE 4 DEFAULT STATE");
      if (prevState == state)
      {
        //Do Nothing
      }
      else
      {
        digitalWrite(LED_RED, LOW);
        digitalWrite(LED_GREEN, LOW);
        digitalWrite(LED_YELLOW, HIGH);
      }
      break;

    case 205: //Easter Egg
      playStarWarsSong();
      break;
    case 500: //Server error
      Serial.println("CASE 500 SERVER ERROR");
      break;
    default: //Invalid state selected
      // statements
      Serial.println("Unknown Error");
      Serial.println("Value of 'state' is " + state);
      break;
    }

    http.end();
    prevState = state;
  }
  
}

void beepLoud()
{
    if(counter % 2 == 0)
  {
    digitalWrite(LED_RED, HIGH);
    delay(200);
    digitalWrite(LED_RED, LOW);
  }else
  {
    digitalWrite(LED_YELLOW, HIGH);
    delay(200);
    digitalWrite(LED_YELLOW, LOW);
  }
  digitalWrite(SPEAKER, HIGH);
  delay(200);
  digitalWrite(SPEAKER, LOW);
  counter++;
}

void playStarWarsSong()
{
 
  //Play first section
  firstSection();
 
  //Play second section
  secondSection();
 
  //Variant 1
  beep(f, 250);  
  beep(gS, 500);  
  beep(f, 350);  
  beep(a, 125);
  beep(cH, 500);
  beep(a, 375);  
  beep(cH, 125);
  beep(eH, 650);
 
  delay(500);
 
  //Repeat second section
  secondSection();
 
  //Variant 2
  beep(f, 250);  
  beep(gS, 500);  
  beep(f, 375);  
  beep(cH, 125);
  beep(a, 500);  
  beep(f, 375);  
  beep(cH, 125);
  beep(a, 650);  
 
  delay(650);
}
 
void beep(int note, int duration)
{
  //Play tone on buzzerPin
  tone(SPEAKER, note, duration);
 
  //Play different LED depending on value of 'counter'
  if(counter % 2 == 0)
  {
    digitalWrite(LED_RED, HIGH);
    delay(duration);
    digitalWrite(LED_RED, LOW);
  }else
  {
    digitalWrite(LED_YELLOW, HIGH);
    delay(duration);
    digitalWrite(LED_YELLOW, LOW);
  }
 
  //Stop tone on buzzerPin
  noTone(SPEAKER);
 
  delay(50);
 
  //Increment counter
  if(counter > 1000)
  {
    counter = 0;
  }
  counter++;
  
}
 
void firstSection()
{
  beep(a, 500);
  beep(a, 500);    
  beep(a, 500);
  beep(f, 350);
  beep(cH, 150);  
  beep(a, 500);
  beep(f, 350);
  beep(cH, 150);
  beep(a, 650);
 
  delay(500);
 
  beep(eH, 500);
  beep(eH, 500);
  beep(eH, 500);  
  beep(fH, 350);
  beep(cH, 150);
  beep(gS, 500);
  beep(f, 350);
  beep(cH, 150);
  beep(a, 650);
 
  delay(500);
}
 
void secondSection()
{
  beep(aH, 500);
  beep(a, 300);
  beep(a, 150);
  beep(aH, 500);
  beep(gSH, 325);
  beep(gH, 175);
  beep(fSH, 125);
  beep(fH, 125);    
  beep(fSH, 250);
 
  delay(325);
 
  beep(aS, 250);
  beep(dSH, 500);
  beep(dH, 325);  
  beep(cSH, 175);  
  beep(cH, 125);  
  beep(b, 125);  
  beep(cH, 250);  
 
  delay(350);
}
