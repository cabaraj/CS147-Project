#include "wireless_connections.h"
#include "functionalities.h"

//BLE - Phone as sender
#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <Wire.h>
#include <SPI.h>
//WiFi
#include <WiFi.h>
#include <HttpClient.h>

#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

//from reading temperature
float temp;
//value received from BLE
std::string value;


// This example downloads the URL "http://arduino.cc/"
char ssid[] = "UCInet Mobile Access"; // your network SSID (name)
char pass[] = "\0"; // your network password (use for WPA, or use as key for WEP)

IPAddress address(18,222,86,137);
uint16_t kPort = 5000;

const int kNetworkTimeout = 30*1000;
// Number of milliseconds to wait if no data is available before trying again
const int kNetworkDelay = 250; // 1000



class MyCallbacks: public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    value = pCharacteristic->getValue();
    if (value.length() > 0) {
      Serial.println("*********");
      Serial.print("New value: ");
      for (int i = 0; i < value.length(); i++)
        Serial.print(value[i]);
        Serial.println();
        Serial.println("*********");
      }
  }
};

void BLE_setup()
{
    Serial.println("1- Download and install an BLE scanner app in your phone");
    Serial.println("2- Scan for BLE devices in the app");
    Serial.println("3- Connect to MyESP32");
    Serial.println("4- Go to CUSTOM CHARACTERISTIC in CUSTOM SERVICE and writesomething");
    Serial.println("5- See the magic =)");
    BLEDevice::init("MyESP32 Group3");
    BLEServer *pServer = BLEDevice::createServer();
    BLEService *pService = pServer->createService(SERVICE_UUID);
    BLECharacteristic *pCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_READ |
        BLECharacteristic::PROPERTY_WRITE 
    );
    pCharacteristic->setCallbacks(new MyCallbacks());
    pCharacteristic->setValue("Hello World");
    pService->start();
    BLEAdvertising *pAdvertising = pServer->getAdvertising();
    pAdvertising->start();
}

void WiFi_setup()
{
    Serial.print("\nConnecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, pass);
    while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.println("MAC address: ");
    Serial.println(WiFi.macAddress());
}

int get_value()
{
    if (value == "ON")
        return 1;
    else if (value == "OFF")
        return 0;
    else
        return 2;
}


void WiFi_loop() {

    int err=0;
    WiFiClient c;
    HttpClient http(c);
    temp = get_temp_F();
    std::string var = "/?var=" + std::to_string(temp);
    const char* var1 = var.c_str();
    //const std::__cxx11::string var1 = var;
    err = http.startRequest("18.222.86.137", 5000, var1, "POST", NULL);
    if (err == 0)
    {
        Serial.println("startedRequest ok");
        err = http.responseStatusCode();
        if (err >= 0)
        {
            Serial.print("Got status code: ");
            Serial.println(err);
            // Usually you'd check that the response code is 200 or a
            // similar "success" code (200-299) before carrying on,
            // but we'll print out whatever response we get
            err = http.skipResponseHeaders();
            if (err >= 0)
            {
                int bodyLen = http.contentLength();
                Serial.print("Content length is: ");
                Serial.println(bodyLen);
                Serial.println();
                Serial.println("Body returned follows:");
                // Now we've got to the body, so we can print it out
                unsigned long timeoutStart = millis();
                char c;
                // Whilst we haven't timed out & haven't reached the end of the body
                while ( (http.connected() && http.available()) &&
                ((millis() - timeoutStart) < kNetworkTimeout) )
                {
                    if (http.available())
                    {
                        c = http.read();
                        // Print out this character
                        Serial.print(c);
                        bodyLen--;
                        // We read something, reset the timeout counter
                        timeoutStart = millis();
                    }
                    else
                    {
                        // We haven't got any data, so let's pause to allow some to
                        // arrive
                        delay(kNetworkDelay);
                    }
                }
            }
            else
            {
                Serial.print("Failed to skip response headers: ");
                Serial.println(err);
            }
        }
        else
        {
            Serial.print("Getting response failed: ");
            Serial.println(err);
        }
    }
    else
    {
        Serial.print("Connect failed: ");
        Serial.println(err);
    }
    http.stop();
}