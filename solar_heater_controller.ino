#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DallasTemperature.h>
#include "LIFOQueue.h"
#include <ESP8266WiFi.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
#define VERSION_NUMBER "2023-05-13"

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const byte relay = D3;
const byte oneWireBus = D7;
String yellowLineString = "";
LiFoQueue queueHeater(20);
LiFoQueue queueBuffer(20);
int temperatureDelta = 5;
const DeviceAddress heaterSensor = {0x28, 0x6C, 0x1E, 0x43, 0x98, 0x0C, 0x00, 0x56};
const DeviceAddress bufferSensor = {0x28, 0x7F, 0x22, 0x43, 0x98, 0x25, 0x00, 0x82};
// setup temperature sensor communication
OneWire oneWire(oneWireBus);
DallasTemperature sensors(&oneWire);

void setup() {
  Serial.begin(9600);
  Serial.println("Begin execution ...");
  WiFi.forceSleepBegin();
  initScreen();
  Serial.println("After init screen ...");
  pinMode(relay, OUTPUT);
  digitalWrite(relay, HIGH);

  sensors.begin();
  //set the resolution to the max
  sensors.setResolution(12);
  Serial.println("Setup ends...");
}

void initScreen(){  
  Serial.println("Init screen...");
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  display.display();
  delay(2000); // Pause for 2 seconds
  display.clearDisplay();
  display.setTextColor(WHITE);
}

void pushToScreen(String messageHeater, String messageBuffer){
  yellowLineString = "H" + messageHeater + "B" + messageBuffer; 
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0, 0);
  display.println(yellowLineString);
  display.setTextSize(4);
  display.setCursor(12, 25);
  display.println(String(queueHeater.getAvarage(),1));
  display.display();
}

void readSensors() {
  sensors.requestTemperatures(); // Send the command to get temperature readings
  Serial.print("sensor 0 reading: ");
  Serial.println(sensors.getTempCByIndex(0));
  Serial.print("sensor 1 reading: ");
  Serial.println(sensors.getTempCByIndex(1));
Serial.println("============================================");
  double valueHeater = sensors.getTempC(heaterSensor);
  double valueBuffer = sensors.getTempC(bufferSensor);
  //sensors.getTempCByIndex(1);
  /*Serial.print("sensor heater reading: ");
  Serial.println(valueHeater);
  Serial.print("sensor buffer reading: ");
  Serial.println(valueBuffer);
  byte readingIndicator = 0;
  //filter out the possible junk values
  if (valueHeater  > -30 && valueHeater < 140) {
    readingIndicator = readingIndicator + 1;
  }
  if (valueBuffer  > -30 && valueBuffer < 140 ) {
    readingIndicator = readingIndicator + 2;
  }
  if (readingIndicator == 3) {
    queueHeater.pushValue(valueHeater);
    queueBuffer.pushValue(valueBuffer);sensors.getTempCByIndex(1)
    pushToScreen(String(valueHeater,1), String(valueBuffer,1));
  } else {
    if (readingIndicator == 1) {
      pushToScreen(String(valueHeater,1), "JK");
      Serial.println("junk values from buffer...");      
    } else {
      if (readingIndicator == 2) {
        pushToScreen("JK", String(valueBuffer,1));
        Serial.println("junk values from heater...");
      }
      else {
        pushToScreen("JK", "JK");
        Serial.println("junk values...");
      }
    }
  }*/
}

void loop() {
  /*Serial.print("==");
  Serial.println(String(onSwitch));*/
  readSensors();
  if (queueHeater.getSize()>=10 && queueBuffer.getSize()>=10) {
   if (queueHeater.getAvarage() >= 85 || queueHeater.getAvarage() - temperatureDelta >= queueBuffer.getAvarage() ) {
      Serial.println("Set optocoupler HIGH");
      digitalWrite(relay, LOW);
    } else {
      Serial.println("Set optocoupler LOW");
      digitalWrite(relay, HIGH);
    }
  }
  /*Serial.print("Queue size: ");
  Serial.print(queue.getSize());
  Serial.print(" | Queue average: ");
  Serial.println(queue.getAvarage());*/
  delay(1000);
}
