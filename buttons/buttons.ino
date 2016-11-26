#include <MRF24G.h>
#include <UbidotsWiFire.h>

#define ID "583395647625424bff431f65"
#define TOKEN "5abqttgRXjaT45IK4cWZ6J4NuSbjVT"
#define SSIDWIFI "ITBA-Labs"
#define PASS "ITBALABS" 
int startButton = 3;
int endButton = 1;
int count = 0;
int startState = 0;
int lastButtonStartState = 0;
long startTime;
int lastButtonEndState = 0;
int endState = 0;

Ubidots client(TOKEN);

void setup() {
  Serial.begin(9600);
  pinMode(startButton,INPUT);
  pinMode(endButton,INPUT);
  client.setWifiConnection(SSIDWIFI, PASS);
}

// Usar DeBounce
void loop() {
  startState = digitalRead(startButton);

  if(lastButtonStartState != startState){
    if(startState == HIGH){
       Serial.println("Started timer");
       startTime = millis();
    }
    delay(50);
  }
  lastButtonStartState = startState;
  endState = !digitalRead(endButton);
  if(lastButtonEndState != endState){
    if(endState == HIGH){
       Serial.print("Elapsed Time: ");
       Serial.print((millis() - startTime) / 1000.0);
       Serial.println(" seconds");
       client.add(ID, (millis() - startTime) / 1000.0);
       client.sendAll();
    }
    delay(50);
  }
  
  lastButtonEndState = endState;
}
