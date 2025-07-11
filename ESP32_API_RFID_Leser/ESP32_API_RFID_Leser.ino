// Created by Jonas Wolf, 2025

#include <ArduinoJson.h>
#include <ArduinoJson.hpp>
#include <WiFi.h>
#include <WebServer.h>
#include <MFRC522.h>

// RFID Reader
#define RST_PIN         0
#define SS_PIN          5 // SDA
MFRC522 mfrc522(SS_PIN, RST_PIN);
// ===

// WLAN
const char* ssid = "SSID";
const char* password = "Password";
// ===

// Webserver
WebServer server(80);
volatile bool isBusy = false;
// ===

// Endpoints
void handleBusy() {
  server.sendHeader("Retry-After", "2");
  server.send(503, "application/json", "{\"error\":\"ESP is busy, try again in 2 Seconds.\"}");
}
void handlePing()
{
  if(isBusy)
  {
    handleBusy();
    return;
  }
  isBusy = true;
  server.send(200, "application/json", "{\"message\":\"Pong\"}");
  isBusy = false;
};
void handleCardPresent()
{
  if(isBusy)
  {
    handleBusy();
    return;
  }
  isBusy = true;
  mfrc522.PCD_Init();
  if(!mfrc522.PICC_IsNewCardPresent()){
    server.send(500, "application/json", "{\"error\":\"No Card recognized\"}");
    isBusy = false;
    return;
  }
  server.send(200, "application/json", "{\"message\":\"Card is recognized\"}");
  isBusy = false;
};
void handleReadUID()
{
  if(isBusy)
  {
    handleBusy();
    return;
  }
  isBusy = true;
  mfrc522.PCD_Init();  // Leser zurücksetzen → wichtig für Wiederholungen!

  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
    server.send(500, "application/json", "{\"error\":\"No Card recognized\"}");
    isBusy = false;
    return;
  }

  String uidString = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    if (mfrc522.uid.uidByte[i] < 0x10) uidString += "0";
    uidString += String(mfrc522.uid.uidByte[i], HEX);
  }

  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();

  server.send(200, "application/json", "{\"uid\":\"" + uidString + "\"}");
  isBusy = false;
};
void handleWriteData()
{
  if (isBusy) {
    handleBusy();
    return;
  }
  isBusy = true;

  if (!server.hasArg("sector") || !server.hasArg("block") || !server.hasArg("data")) {
    server.send(400, "application/json", "{\"error\":\"Missing Parameters\"}");
    isBusy = false;
    return;
  }

  int sector = server.arg("sector").toInt();
  int block = server.arg("block").toInt();
  String dataStr = server.arg("data");

  if (sector < 1 || sector > 15 || block < 0 || block > 2) {
    server.send(400, "application/json", "{\"error\":\"Invalid Sektor (1-15) or Block(0-2)\"}");
    isBusy = false;
    return;
  }

  if (dataStr.length() > 32) { // 16 Bytes = 32 Hex-Zeichen
    server.send(400, "application/json", "{\"error\":\"Data to long, max. Length 16 Bytes\"}");
    isBusy = false;
    return;
  }

  mfrc522.PCD_Init();
  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
    server.send(500, "application/json", "{\"error\":\"No Card recognized\"}");
    isBusy = false;
    return;
  }

  // Hex-String in Byte-Array umwandeln
  byte blockData[16];
  for (int i = 0; i < 16; i++) {
    if (i*2 + 1 < dataStr.length()) {
      String byteStr = dataStr.substring(i*2, i*2 + 2);
      blockData[i] = (byte) strtol(byteStr.c_str(), NULL, 16);
    } else {
      blockData[i] = 0x00; // auffüllen mit 0
    }
  }

  byte blockAddr = sector * 4 + block;
  MFRC522::MIFARE_Key key;
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;

  MFRC522::StatusCode status = mfrc522.PCD_Authenticate(
    MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockAddr, &key, &(mfrc522.uid)
  );

  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Authentifizierung fehlgeschlagen: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    server.send(500, "application/json", "{\"error\":\"Authentification failed\"}");
    isBusy = false;
    return;
  }

  status = mfrc522.MIFARE_Write(blockAddr, blockData, 16);
  if (status != MFRC522::STATUS_OK) {
    server.send(500, "application/json", "{\"error\":\"Writing failed\"}");
  } else {
    server.send(200, "application/json", "{\"message\":\"Data was written successfully\"}");
  }

  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
  isBusy = false;
};
void handleReadData() {
  if (isBusy) {
    handleBusy();
    return;
  }
  isBusy = true;

  int sector = server.arg("sector").toInt();
  int block = server.arg("block").toInt();

  if (sector < 1 || sector > 15 || block < 0 || block > 2) {
    server.send(400, "application/json", "{\"error\":\"Invalid Sektor (1-15) or Block(0-2)\"}");
    isBusy = false;
    return;
  }

  mfrc522.PCD_Init();
  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
    server.send(500, "application/json", "{\"error\":\"No Card recognized\"}");
    isBusy = false;
    return;
  }

  byte blockAddr = sector * 4 + block;
  MFRC522::MIFARE_Key key;
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;

  MFRC522::StatusCode status = mfrc522.PCD_Authenticate(
    MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockAddr, &key, &(mfrc522.uid)
  );

  if (status != MFRC522::STATUS_OK) {
    server.send(500, "application/json", "{\"error\":\"Authentification failed\"}");
    isBusy = false;
    return;
  }

  byte buffer[18]; // 16 Bytes + 2 Bytes CRC
  byte size = sizeof(buffer);
  status = mfrc522.MIFARE_Read(blockAddr, buffer, &size);
  if (status != MFRC522::STATUS_OK) {
    server.send(500, "application/json", "{\"error\":\"Reading failed\"}");
    isBusy = false;
    return;
  }

  // Ausgabe als Hex-String
  String dataStr = "";
  for (byte i = 0; i < 16; i++) {
    if (buffer[i] < 0x10) dataStr += "0";
    dataStr += String(buffer[i], HEX);
  }

  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();

  server.send(200, "application/json", "{\"data\":\"" + dataStr + "\"}");
  isBusy = false;
}
// ===

void setup() {
  Serial.begin(115200);
  while (!Serial);
  
  SPI.begin();
	mfrc522.PCD_Init();

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WLAN");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected! IP-Address: " + WiFi.localIP().toString());

  server.on("/api/alive", HTTP_GET, handlePing);
  server.on("/rfid/cardPresent", HTTP_GET, handleCardPresent);
  server.on("/rfid/readUID", HTTP_GET, handleReadUID);
  server.on("/rfid/writeData", HTTP_POST, handleWriteData);
  server.on("/rfid/readData", HTTP_GET, handleReadData);

  server.begin();
  Serial.println("HTTP Server gestartet");
}

void loop() {
  server.handleClient();
}
