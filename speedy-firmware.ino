#include <ArduinoJson.h>
#include <Servo.h>
#include <EEPROM.h>
#include <dht.h>

// Feeder Servo
Servo feeder;
const int feederPin = 14;
const int feederIncrement = 36;
const int feederLastPosition = 0;
int feederPosition = 0;

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
  feederPosition = EEPROM.read(feederLastPosition);
  setFeederPosition();

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
    feedNow();
    printFeederStatus();
    return;
  }

  if (command == "feeder.reset") {
    feederReset();
    printFeederStatus();
    return;
  }

  if (command == "feeder.status") {
    printFeederStatus();
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

void printFeederStatus() {
  feederStatus["remaining_feeds"] = (180 - feederPosition) / feederIncrement;
  feederStatus["position"] = feederPosition;
  feederStatus.prettyPrintTo(Serial);
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

void feedNow() {
  feederPosition += feederIncrement;
  if (feederPosition > 180) {
    feederPosition = 180;
    return;
  }
  setFeederPosition();
}

void setFeederPosition() {
  if (feederPosition > 180) return;

  feeder.attach(feederPin);
  feeder.write(feederPosition);
  delay(1000);
  feeder.detach();
  EEPROM.write(feederLastPosition, feederPosition);
}

void feederReset() {
  feederPosition = 0;
  setFeederPosition();
}

int digitalReadOutputPin(uint8_t pin) {
  uint8_t bit = digitalPinToBitMask(pin);
  uint8_t port = digitalPinToPort(pin);
  
  if (port == NOT_A_PIN) return LOW;
  
  return (*portOutputRegister(port) & bit) ? HIGH : LOW;
}

