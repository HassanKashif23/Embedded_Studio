#include <WiFi.h>
#include <esp_now.h>
#include <SPI.h>
#include "DW1000Ranging.h"
#include "DW1000.h"

// Pin definitions for SPI and DW1000
#define SPI_SCK 18
#define SPI_MISO 19
#define SPI_MOSI 23
#define DW_CS 4

// ESP-NOW broadcast address
uint8_t broadcastAddress[] = {0xD4, 0xD4, 0xDA, 0x46, 0x6C, 0xD8};

// Connection pins
const uint8_t PIN_RST = 27; // reset pin
const uint8_t PIN_IRQ = 34; // irq pin
const uint8_t PIN_SS = 4;   // spi select pin

String input;

// TAG antenna delay defaults to 16384
// leftmost two bytes below will become the "short address"
char tag_addr[] = "7D:00:22:EA:82:60:3B:9C";

typedef struct struct_message {
  char msg[128];
} struct_message;

// Create a struct_message called myData
struct_message myData;
esp_now_peer_info_t peerInfo;

// Callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void setup() {
  // Debug console
  Serial.begin(115200);
  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
  DW1000Ranging.initCommunication(PIN_RST, PIN_SS, PIN_IRQ); // Reset, CS, IRQ pin
  DW1000Ranging.attachNewRange(newRange);
  DW1000Ranging.attachNewDevice(newDevice);
  DW1000Ranging.attachInactiveDevice(inactiveDevice);

  // Start as tag, do not assign random short address
  DW1000Ranging.startAsTag(tag_addr, DW1000.MODE_LONGDATA_RANGE_LOWPOWER, false);

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_register_send_cb(OnDataSent);

  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;

  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }

  Serial.println("Enter '1' to send ESP-NOW message, '0' to read UWB range.");

  // Send initial message via ESP-NOW
  strcpy(myData.msg, "THIS IS A CHAR");
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
  if (result == ESP_OK) {
    Serial.println("Sent with success");
  } else {
    Serial.println("Error sending the data");
  }
}

void loop() {
  // Handle UWB ranging loop
  DW1000Ranging.loop();

  // Check for user input
  if (Serial.available()) {
    input = Serial.readStringUntil('\n');
    input.trim(); // Remove any leading or trailing whitespace
    Serial.println("Serial input received");

    // Handle user commands
    if (input == "1") {
      Serial.println("data");
      strcpy(myData.msg, "THIS IS A CHAR");
      esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
      if (result == ESP_OK) {
        Serial.println("ESP-NOW: Sent with success");
      } else {
        Serial.println("ESP-NOW: Error sending the data");
      }
    } else if (input == "0") {
      Serial.println("Reading UWB range...");
      DW1000Ranging.loop();
      // UWB range is handled in the DW1000Ranging.loop()
    }
  }
}

void newRange() {
  Serial.print("from: ");
  Serial.print(DW1000Ranging.getDistantDevice()->getShortAddress(), HEX);
  Serial.print(",\t Range: ");
  Serial.print(DW1000Ranging.getDistantDevice()->getRange());
  Serial.println(" m");
  delay(2000);
}

void newDevice(DW1000Device *device) {
  Serial.print("Device added: ");
  Serial.println(device->getShortAddress(), HEX);
}

void inactiveDevice(DW1000Device *device) {
  Serial.print("Delete inactive device: ");
  Serial.println(device->getShortAddress(), HEX);
}
