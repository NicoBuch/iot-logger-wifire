#include <WiFi.h>
#include <SPI.h>
#include "RestClient.h"


char *ssid = "ITBA-Labs";
char *pass = "ITBALABS";
char *host =  "10.16.35.55";
int port = 4567;
int status = WL_IDLE_STATUS; 


RestClient client = RestClient(host, port, ssid, pass);


String user = "nicobuchhalter"; // This is the only thing that should be needed to be configured
String card_id = "572a275a7e6673b398bf75a2";

#define LED_PIN 13
#define START_BUTTON 5
#define END_BUTTON 3
#define POT A0
#define OK_BUTTON 8 // This is to select the correct board or card

unsigned long startTime;

int lastButtonStartState = LOW;

int lastButtonEndState = LOW;

int seconds;

const int maxPot = 4095;
const int potBounce = 10;

int currentPot;
int lastPot = 0;
String response = "holis";

char * boards[3] = {"board1", "board2", "board3"}; // The idea is to request the API for this data and with the POT choose.
char * cards[3] = {"card1", "card2", "card3"};

void blinkForever()
{
    for (;;)
    {
        digitalWrite(LED_PIN, HIGH);
        delay(500);
        digitalWrite(LED_PIN, LOW);
        delay(500);   
    }
}

void setup()
{
    Serial.begin(9600);
    Serial.println("IOT Logger Client 1.0");
    Serial.println("");
    
    pinMode(LED_PIN, OUTPUT);
    pinMode(START_BUTTON,INPUT);
    pinMode(END_BUTTON,INPUT);
    pinMode(OK_BUTTON, INPUT);
    pinMode(POT, INPUT);
    client.connect();
    
    Serial.println("Connecting to WiFi...");
}




void loop()
{ 
  if(button_pressed(&lastButtonStartState, START_BUTTON)){
    Serial.println("Started timer");
    startTime = millis();
  }
  if(button_pressed(&lastButtonEndState, END_BUTTON)){
    seconds = (millis() - startTime) / 1000;
    generate_log();
  }
}



bool button_pressed(int* lastButtonState, int button){
  int state = digitalRead(button);
  
  bool answer = false;
  if(*lastButtonState != state){
      if(state == HIGH){
         answer = true;
      }
  }
  *lastButtonState = state;
  
  return answer;
  
}

void generate_log(){
  String path = "/log?card_id=";
  String seconds_param = "seconds&";
  const char * request = (path + card_id + seconds_param + seconds).c_str(); 
  client.get(request, &response);
  Serial.print("Response: ");
  Serial.println(response);
}

