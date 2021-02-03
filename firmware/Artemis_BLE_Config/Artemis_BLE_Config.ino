#include "ArduinoBLE.h"

const byte LED_TO_TOGGLE = 19;

BLEService olaLEDService("4fafc201-1fb5-459e-8fcc-c5c9c331914b");

BLEUnsignedIntCharacteristic olaIntValueChar("beb5483e-36e1-4688-b7f5-ea07361b26a8", BLERead | BLENotify | BLEWrite);
BLEDescriptor olaIntValueDescriptor("beb5483e-36e1-4688-b7f5-ea07361b26a8", "OLA Int Value");

void setup() {

  // start up serial port
  Serial.begin(115200);
  while (!Serial);

  // led to display when connected
  pinMode(LED_BUILTIN, OUTPUT);

  // start BLE
  if ( ! BLE.begin())
  {
    Serial.println("starting BLE failed!");
    while (1);
  }

  // name the device
  BLE.setLocalName("OpenLog");
  BLE.setAdvertisedService(olaLEDService);

  // setup int char
  olaIntValueChar.addDescriptor(olaIntValueDescriptor);
  olaLEDService.addCharacteristic(olaIntValueChar);
  olaIntValueChar.setValue(152);

  // add the LED service to the BLE device
  BLE.addService(olaLEDService);

  // broadcast BLE connection
  BLE.advertise();

  Serial.println(F("OLA BLE ready for connections!"));
}

void loop()
{
  BLEDevice central = BLE.central();

  // we got a connection!
  if (central)
  {
    Serial.print("Connected to central: ");
    Serial.println(central.address());
    digitalWrite(LED_BUILTIN, HIGH);

    while (central.connected()) {

      if (olaIntValueChar.written()) {
        Serial.println("Value written to Int characteristic:");
        Serial.println(olaIntValueChar.value());
      }

      delay(200);
    }
    
    digitalWrite(LED_BUILTIN, LOW);
    Serial.print("Disconnected from central: ");
    Serial.println(central.address());
  }

}
