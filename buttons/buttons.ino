

/*
 * iot-logger - WiFire Sketch
 *
 * Based on: https://github.com/ricklon/deIPcK/blob/master/DEWFcK/examples/WiFiTCPEchoClient/WiFiTCPEchoClient.pde
 */

#include <MRF24G.h>
#include <DEIPcK.h>
#include <DEWFcK.h>

// CONSTANTS

// SSID
const char *szSsid = "nicobuch";

// select 1 for the security you want, or none for no security
#define USE_WPA2_PASSPHRASE
//#define USE_WPA2_KEY
//#define USE_WEP40
//#define USE_WEP104
//#define USE_WF_CONFIG_H

// modify the security key to what you have.
#if defined(USE_WPA2_PASSPHRASE)

    const char * szPassPhrase = "lavalleja1524c";
    #define WiFiConnectMacro() deIPcK.wfConnect(szSsid, szPassPhrase, &status)

#elif defined(USE_WPA2_KEY)

    DEWFcK::WPA2KEY key = { 0x27, 0x2C, 0x89, 0xCC, 0xE9, 0x56, 0x31, 0x1E, 
                            0x3B, 0xAD, 0x79, 0xF7, 0x1D, 0xC4, 0xB9, 0x05, 
                            0x7A, 0x34, 0x4C, 0x3E, 0xB5, 0xFA, 0x38, 0xC2, 
                            0x0F, 0x0A, 0xB0, 0x90, 0xDC, 0x62, 0xAD, 0x58 };
    #define WiFiConnectMacro() deIPcK.wfConnect(szSsid, key, &status)

#elif defined(USE_WEP40)

    const int iWEPKey = 0;
    DEWFcK::WEP40KEY keySet = {    0xBE, 0xC9, 0x58, 0x06, 0x97,     // Key 0
                                    0x00, 0x00, 0x00, 0x00, 0x00,     // Key 1
                                    0x00, 0x00, 0x00, 0x00, 0x00,     // Key 2
                                    0x00, 0x00, 0x00, 0x00, 0x00 };   // Key 3
    #define WiFiConnectMacro() deIPcK.wfConnect(szSsid, keySet, iWEPKey, &status)

#elif defined(USE_WEP104)

    const int iWEPKey = 0;
    DEWFcK::WEP104KEY keySet = {   0x3E, 0xCD, 0x30, 0xB2, 0x55, 0x2D, 0x3C, 0x50, 0x52, 0x71, 0xE8, 0x83, 0x91,   // Key 0
                                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   // Key 1
                                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   // Key 2
                                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // Key 3
    #define WiFiConnectMacro() deIPcK.wfConnect(szSsid, keySet, iWEPKey, &status)

#elif defined(USE_WF_CONFIG_H)

    #define WiFiConnectMacro() deIPcK.wfConnect(0, &status)

#else   // no security - OPEN

    #define WiFiConnectMacro() deIPcK.wfConnect(szSsid, &status)

#endif

typedef enum
{
    NONE = 0,
    CONNECT,
    TCPCONNECT,
    WRITE,
    READ,
    CLOSE,
    ERR
} STATE;

STATE state = CONNECT;
IPSTATUS    status;

unsigned tStart = 0;
unsigned tWait = 5000;

TCPSocket tcpSocket;
byte rgbRead[1024];
int cbRead = 0;




// IOT Logger config

const char *loggerServerIp = "192.168.0.6";
const uint16_t loggerServerPort = 4567;

const char *user = "nicobuchhalter"; // This is the only thing that should be needed to be configured
const char * card_id = "572a275a7e6673b398bf75a2";

#define LED_PIN 13
#define START_BUTTON 3
#define END_BUTTON 5

unsigned long startTime;

int startState;
int lastButtonStartState = LOW;
unsigned long lastDebounceStartTime = 0;  // the last time the output pin was toggled

int lastButtonEndState = LOW;
int endState;

unsigned long lastDebounceEndTime = 0;  // the last time the output pin was toggled

unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers


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

    Serial.println("Connecting to WiFi...");
}




void loop()
{ 
    int cbRead = 0;

    switch(state)
    {
        case CONNECT:
            delay(100);
            Serial.println("CONNECT");
            if (WiFiConnectMacro())
            {
                Serial.println("WiFi connected");
                deIPcK.begin();
                state = TCPCONNECT;
            }
            else if (IsIPStatusAnError(status))
            {
                Serial.print("Unable to connect, status: ");
                Serial.println(status, DEC);
                state = ERR;
            }
            break;

        case TCPCONNECT:
            delay(100);
            Serial.print("TCPCONNECT");
            Serial.print("  ");
            Serial.print(loggerServerIp);
            Serial.print(":");
            Serial.println(loggerServerPort);
            if (deIPcK.tcpConnect(loggerServerIp, loggerServerPort, tcpSocket, &status))
            {
                Serial.println("Connected to server.");
                state = WRITE;
            }
            else {
              Serial.print("Status: ");
              Serial.println(status);
            }
            break;

        case WRITE:
            delay(100);
            Serial.println("WRITE");
            int seconds, readingStart, readingEnd;
            if (tcpSocket.isEstablished())
            {
                  // Debounce algorithm from https://www.arduino.cc/en/Tutorial/Debounce
                  readingStart = digitalRead(START_BUTTON);

                  if(lastButtonStartState != readingStart){
                    lastDebounceStartTime = millis();
                  }

                  if((millis() - lastDebounceStartTime) > debounceDelay){
                    if(readingStart != startState){
                      startState = readingStart;
                    }
                    
                    if(startState == HIGH){
                       Serial.println("Started timer");
                       startTime = millis();
                    }
                  }
                  lastButtonStartState = readingStart;

                  
                  readingEnd = digitalRead(END_BUTTON);
                  if(lastButtonEndState != readingEnd){
                    lastDebounceEndTime = millis();
                  }

                  if((millis() - lastDebounceEndTime) > debounceDelay){
                    if(readingEnd != endState){
                      endState = readingEnd;
                    }
                    
                    if(endState == HIGH){
                       seconds = (millis() - startTime) / 1000.0;
                        tcpSocket.print("GET /log?seconds=");
                        tcpSocket.print(seconds);
                        tcpSocket.print("&card_id=");
                        tcpSocket.print(card_id);
                        tcpSocket.print(" HTTP/1.1\r\n");
                        tcpSocket.print("Host: www.iot-logger.com\r\n");
                        tcpSocket.print("Content-Type: application/json\r\n");
                        tcpSocket.print("Accept: */*\r\n");
                        tcpSocket.print("\r\n");
                        
                        Serial.println();
                        Serial.println("Bytes Read Back:");
                        state = READ;
                        tStart = (unsigned) millis();
                    }
                  }
                  lastButtonEndState = readingEnd;
            }
            break;

            case READ:
                delay(100);
                Serial.println("READ");
                if ((cbRead = tcpSocket.available()) > 0)
                {
                    cbRead = cbRead < sizeof(rgbRead) ? cbRead : sizeof(rgbRead);
                    cbRead = tcpSocket.readStream(rgbRead, cbRead);

                    for (int i = 0; i < cbRead; i++)
                    {
                        Serial.print((char)rgbRead[i]);
                    }
                }
                else if ((((unsigned)millis()) - tStart) > tWait)
                {
                    state = CLOSE;
                }

                break;

        case CLOSE:
            Serial.println("CLOSE");
            tcpSocket.close();
            Serial.println("Closing TCP Socket.");
            state = TCPCONNECT;
            break;

        case ERR:
        default:
            Serial.println("Entering error state.");
            blinkForever();
            break;
    }

    DEIPcK::periodicTasks();
}
