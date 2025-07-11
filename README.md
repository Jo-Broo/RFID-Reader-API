# ESP32 RFID API

This project allows an ESP32 equipped with an MFRC522 RFID reader to read from and write to MIFARE Classic 1K cards via a simple HTTP API. The goal is to interact with RFID tags over WiFi using HTTP requests for reading and writing data.

## Features

- Write 16 bytes of hex-encoded data to a specific sector and block
- Read data from a specific sector and block
- Minimal web API for integration and testing
- Busy state handling to avoid overlapping operations
- Uses default Key A (`FF FF FF FF FF FF`) for authentication

## Hardware Required

- ESP32 (e.g. DevKit v1)
- MFRC522 RFID reader module
- MIFARE Classic 1K RFID card/tag
- Jumper wires
- Optional: USB power supply

## Wiring (ESP32 ↔ MFRC522)

| MFRC522 Pin | ESP32 Pin |
|-------------|-----------|
| SDA         | D5        |
| SCK         | D18       |
| MOSI        | D23       |
| MISO        | D19       |
| RST         | D21       |
| GND         | GND       |
| 3.3V        | 3.3V      |

> Note: Do **not** power the MFRC522 with 5V — use 3.3V only.

## API Endpoints

### Alive  
| Field       | Value                        |
|-------------|------------------------------|
| Method      | GET                          |
| Path        | /api/alive                   |
| Description | Returns a Pong to your Ping. |
| Format      | application/json             |
| Value       | {"message":"Pong"}           |

### Card Present
| Field         | Value                                     |
|---------------|-------------------------------------------|
| Method        | GET                                       |
| Path          | /rfid/cardPresent                         |
| Description   | Returns if the RFID Reader detects a Chip |
| Format        | application/json                          |
| Value Success | {"message":"Card is recognized"}          |
| Value Failure | {"error":"No Card recognized"}            |

### Read UID
| Field         | Value                                          |
|---------------|------------------------------------------------|
| Method        | GET                                            |
| Path          | /rfid/readUID                                  |
| Description   | Reads the UID of a RFID Chip if one is present |
| Format        | application/json                               |
| Value Success | {"uid":"(uid of the Chip)"}                    |
| Value Failure | {"error":"No Card recognized"}                 |

### Write Data
| Field         | Value                                                                                                                    |
|---------------|--------------------------------------------------------------------------------------------------------------------------|
| Method        | POST                                                                                                                     |
| Path          | /rfid/writeData                                                                                                          |
| Parameter     | **sector**: Data Sector, **block**: Data Block, **data**: Data                                                           |
| Description   | Writes the specified Data to the specified Datablock.<br>The block Authentification uses the standard Key 0xFFFFFFFFFFFF |
| Format        | application/json                                                                                                         |
| Value Success | {"message":"Data was written successfully"}                                                                              |
| Value Failure | {"error":"No Card recognized"}                                                                                           |
| Value Failure | {"error":"Authentification failed"}                                                                                      |
| Value Failure | {"error":"Writing failed"}                                                                                               |

### Read Data
| Field         | Value                                                                                                                    |
|---------------|--------------------------------------------------------------------------------------------------------------------------|
| Method        | GET                                                                                                                      |
| Path          | /rfid/readData                                                                                                           |
| Parameter     | **sector**: Data Sector, **block**: Data Block                                                                           |
| Description   | Reads Data from the specified Datablock.<br>The block Authentification uses the standard Key 0xFFFFFFFFFFFF              |
| Format        | application/json                                                                                                         |
| Value Success | {"data":"(16-Byte Data)"}                                                                                                |
| Value Failure | {"error":"No Card recognized"}                                                                                           |
| Value Failure | {"error":"Authentification failed"}                                                                                      |
| Value Failure | {"error":"Reading failed"}                                                                                               |