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

void pushToScreen(String messageHeater){
  if (yellowLineString.length() >= 9) {
    yellowLineString = yellowLineString.substring(yellowLineString.lastIndexOf("|")-4) + messageHeater + "|"; 
  } else {
    yellowLineString = yellowLineString + "|" + messageHeater;
  }
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
  double valueHeater = sensors.getTempCByIndex(0);
  Serial.print("sensor reading: ");
  Serial.println(valueHeater);
  //filter out the possible junk values
  if (valueHeater > -30 && valueHeater < 140) {
    queueHeater.pushValue(valueHeater);
    pushToScreen(String(valueHeater,1));
  } else {
    pushToScreen("-NA-");
  }
}

void loop() {
  /*Serial.print("==");
  Serial.println(String(onSwitch));*/
  readSensors();
  if (queueHeater.getSize() >= 10) {
    if (queueHeater.getAvarage() >= 65) {
      Serial.println("Above the required value! Set optocoupler HIGH");
      digitalWrite(relay, LOW);
    }
    if (queueHeater.getAvarage() <= 55) {
      Serial.println("Under the minimum value! Set optocoupler HIGH");
      digitalWrite(relay, HIGH);
    }
  } else {
      Serial.println("Not enough data!Set optocoupler LOW to avoid any damage!");
      digitalWrite(relay, LOW);
  }
  /*Serial.print("queueHeater size: ");
  Serial.print(queueHeater.getSize());
  Serial.print(" | queueHeater average: ");
  Serial.println(queueHeater.getAvarage());*/
  delay(5000);
}
