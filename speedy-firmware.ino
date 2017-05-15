#include <ArduinoJson.h>
#include <Servo.h>
#include <EEPROM.h>
#include <dht.h>

// Feeder Servo
Servo feeder;
const int feederPin = 14;
const int feederForward = 0;
const int feederStop = 6;
const int feedingTime = 2000;

// Temperature & humidity sensor
dht DHT;
const int dhtPin = 12;
const int dhtPower = 13;

// UV Sensor
const int uvPin = 17;
const int uvGround = 18;
const int uvPower = 19;

// Lighting Relays
const int lightingPin = 7;
const int lightingLastState = 1;

// Board statuses
const String BOARD_ID = "speedy";
StaticJsonBuffer<200> jsonBuffer;
JsonObject& jsonId = jsonBuffer.createObject();
JsonObject& lightingStatus = jsonBuffer.createObject();
JsonObject& feederStatus = jsonBuffer.createObject();



void setup() {
  pinMode(dhtPower, OUTPUT);
  digitalWrite(dhtPower, HIGH);

  pinMode(uvGround, OUTPUT);
  pinMode(uvPower, OUTPUT);
  digitalWrite(uvGround, LOW);
  digitalWrite(uvPower, HIGH);
  pinMode(uvPin, INPUT);

  pinMode(lightingPin, OUTPUT);
  digitalWrite(lightingPin, EEPROM.read(lightingLastState));
  
  Serial.begin(9600);
  Serial.println("{\"status\":\"ready\"}");
}

void loop() {
  String command = Serial.readStringUntil(';');

  if (command == "ident" || command == "id") {
    printBoardId();
    return;
  }
  
  if (command == "feeder.feed") {
    feed();
    Serial.println("{\"action\":\"feed\", \"status\":\"done\"}");
    return;
  }

  if (command == "feeder.on") {
    feeder.attach(feederPin);
    feeder.write(feederForward);
    return;
  }

  if (command == "feeder.off") {
    feeder.write(feederStop);
    feeder.detach();
    return;
  }

  if (command == "lights.status") {
    printLightingStatus();
    return;
  }

  if (command == "lights.off") {
    digitalWrite(lightingPin, HIGH);
    EEPROM.write(lightingLastState, 1);
    printLightingStatus();
    return;
  }

  if (command == "lights.on") {
    digitalWrite(lightingPin, LOW);
    EEPROM.write(lightingLastState, 0);
    printLightingStatus();
    return;
  }
}

void printLightingStatus() {
  int chk = DHT.read11(dhtPin);
  
  lightingStatus["power"] = !digitalReadOutputPin(lightingPin);
  lightingStatus["uv"] = analogRead(uvPin);
  lightingStatus["temperature"] = DHT.temperature;
  lightingStatus["humidity"] = DHT.humidity;
  lightingStatus.prettyPrintTo(Serial);
}

void printBoardId() {
  jsonId["id"] = BOARD_ID;
  jsonId.prettyPrintTo(Serial);
}

void feed() {
  feeder.attach(feederPin);
  feeder.write(feederForward);
  delay(feedingTime);
  feeder.write(feederStop);
  feeder.detach();
}

void runFeeder(int rotation, int duration) {
  feeder.write(rotation);
  delay(duration);
  feeder.write(feederStop);
}

int digitalReadOutputPin(uint8_t pin) {
  uint8_t bit = digitalPinToBitMask(pin);
  uint8_t port = digitalPinToPort(pin);
  
  if (port == NOT_A_PIN) return LOW;
  
  return (*portOutputRegister(port) & bit) ? HIGH : LOW;
}

