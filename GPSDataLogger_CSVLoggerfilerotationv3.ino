#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include <TinyGPS++.h>

// ============================================================
// GPS Configuration
// ============================================================

#define GPS_RX_PIN 16
#define GPS_TX_PIN 17
#define GPS_BAUD   9600

HardwareSerial GPS(1);
TinyGPSPlus gps;


// ============================================================
// SD Card Configuration
// ============================================================

#define SD_CS_PIN 5

// Maximum file size
// 1 MB
#define MAX_FILE_SIZE (1024UL * 1024UL)

File logFile;


// ============================================================
// Logging Configuration
// ============================================================

#define LOG_INTERVAL   1000    // Log every 1 second
#define FLUSH_INTERVAL 5000    // Flush every 5 seconds

unsigned long lastLogTime = 0;
unsigned long lastFlushTime = 0;


// ============================================================
// Create New GPS Log File
// ============================================================

bool createNewLogFile()
{
    if (!gps.date.isValid() || !gps.time.isValid())
    {
        Serial.println("[GPS] Date/time not valid. Waiting...");
        return false;
    }

    char filename[50];

    snprintf(
        filename,
        sizeof(filename),
        "/GPS_%04d%02d%02d_%02d%02d%02d.csv",
        gps.date.year(),
        gps.date.month(),
        gps.date.day(),
        gps.time.hour(),
        gps.time.minute(),
        gps.time.second()
    );

    Serial.println();
    Serial.print("[SD] Creating new file: ");
    Serial.println(filename);


    // --------------------------------------------------------
    // Check for filename collision
    // --------------------------------------------------------

    if (SD.exists(filename))
    {
        char uniqueFilename[50];

        for (int i = 1; i <= 99; i++)
        {
            snprintf(
                uniqueFilename,
                sizeof(uniqueFilename),
                "/GPS_%04d%02d%02d_%02d%02d%02d_%d.csv",
                gps.date.year(),
                gps.date.month(),
                gps.date.day(),
                gps.time.hour(),
                gps.time.minute(),
                gps.time.second(),
                i
            );

            if (!SD.exists(uniqueFilename))
            {
                strcpy(filename, uniqueFilename);
                break;
            }
        }
    }


    // --------------------------------------------------------
    // Open file
    // --------------------------------------------------------

    logFile = SD.open(filename, FILE_WRITE);

    if (!logFile)
    {
        Serial.println("[ERROR] Failed to create GPS log file.");
        return false;
    }


    // --------------------------------------------------------
    // CSV Header
    // --------------------------------------------------------

    logFile.println(
        "Date,Time,Latitude,Longitude,Altitude_m,Speed_kmh,Satellites,HDOP"
    );

    logFile.flush();


    Serial.println("[SD] New GPS log file created.");
    Serial.println("[SD] CSV header written.");

    lastFlushTime = millis();

    return true;
}


// ============================================================
// Close Current File
// ============================================================

void closeCurrentFile()
{
    if (logFile)
    {
        logFile.flush();
        logFile.close();

        Serial.println("[SD] Current GPS file closed.");
    }
}


// ============================================================
// Setup
// ============================================================

void setup()
{
    Serial.begin(115200);

    delay(1000);

    Serial.println();
    Serial.println("======================================");
    Serial.println(" ESP32 GPS Data Logger - Part 2");
    Serial.println(" CSV Logger + File Rotation");
    Serial.println("======================================");


    // ========================================================
    // Start GPS
    // ========================================================

    GPS.begin(
        GPS_BAUD,
        SERIAL_8N1,
        GPS_RX_PIN,
        GPS_TX_PIN
    );

    Serial.println("[GPS] GPS serial started.");

    Serial.print("[GPS] RX: GPIO ");
    Serial.println(GPS_RX_PIN);

    Serial.print("[GPS] TX: GPIO ");
    Serial.println(GPS_TX_PIN);


    // ========================================================
    // Start SD Card
    // ========================================================

    Serial.println();
    Serial.println("[SD] Initializing SD card...");

    if (!SD.begin(SD_CS_PIN))
    {
        Serial.println("[ERROR] SD card initialization failed!");
        return;
    }

    Serial.println("[SD] SD card initialized successfully.");


    // ========================================================
    // Display SD Card Information
    // ========================================================

    uint8_t cardType = SD.cardType();

    Serial.print("[SD] Card type: ");

    if (cardType == CARD_MMC)
    {
        Serial.println("MMC");
    }
    else if (cardType == CARD_SD)
    {
        Serial.println("SDSC");
    }
    else if (cardType == CARD_SDHC)
    {
        Serial.println("SDHC");
    }
    else
    {
        Serial.println("UNKNOWN");
    }


    Serial.print("[SD] Card size: ");

    Serial.print(
        SD.cardSize() / (1024 * 1024)
    );

    Serial.println(" MB");


    // ========================================================
    // Wait for GPS
    // ========================================================

    Serial.println();
    Serial.println("[GPS] Waiting for valid GPS date/time...");
    Serial.println("[GPS] Waiting for GPS fix...");
}


// ============================================================
// Main Loop
// ============================================================

void loop()
{
    // ========================================================
    // Read GPS Data
    // ========================================================

    while (GPS.available())
    {
        gps.encode(GPS.read());
    }


    // ========================================================
    // Create First File
    // ========================================================

    if (!logFile)
    {
        if (gps.date.isValid() &&
            gps.time.isValid())
        {
            createNewLogFile();
        }
    }


    // ========================================================
    // Log GPS Data Every 1 Second
    // ========================================================

    if (millis() - lastLogTime >= LOG_INTERVAL)
    {
        lastLogTime = millis();


        // ----------------------------------------------------
        // Check GPS Position
        // ----------------------------------------------------

        if (gps.location.isValid())
        {
            // Make sure file exists
            if (!logFile)
            {
                if (!createNewLogFile())
                {
                    return;
                }
            }


            // =================================================
            // Date
            // =================================================

            char dateBuffer[20];

            snprintf(
                dateBuffer,
                sizeof(dateBuffer),
                "%04d-%02d-%02d",
                gps.date.year(),
                gps.date.month(),
                gps.date.day()
            );


            // =================================================
            // Time
            // =================================================

            char timeBuffer[20];

            snprintf(
                timeBuffer,
                sizeof(timeBuffer),
                "%02d:%02d:%02d",
                gps.time.hour(),
                gps.time.minute(),
                gps.time.second()
            );


            // =================================================
            // Write CSV Record
            // =================================================

            logFile.print(dateBuffer);
            logFile.print(",");

            logFile.print(timeBuffer);
            logFile.print(",");

            // Latitude
            logFile.print(
                gps.location.lat(),
                6
            );

            logFile.print(",");

            // Longitude
            logFile.print(
                gps.location.lng(),
                6
            );

            logFile.print(",");

            // Altitude in meters
            logFile.print(
                gps.altitude.meters(),
                2
            );

            logFile.print(",");

            // Speed in km/h
            logFile.print(
                gps.speed.kmph(),
                2
            );

            logFile.print(",");

            // Satellites
            logFile.print(
                gps.satellites.value()
            );

            logFile.print(",");

            // HDOP
            logFile.println(
                gps.hdop.hdop(),
                2
            );


            // =================================================
            // Serial Monitor Output
            // =================================================

            Serial.print("GPS: ");

            Serial.print(
                gps.location.lat(),
                6
            );

            Serial.print(", ");

            Serial.print(
                gps.location.lng(),
                6
            );

            Serial.print(" | Alt: ");

            Serial.print(
                gps.altitude.meters(),
                2
            );

            Serial.print(" m | Speed: ");

            Serial.print(
                gps.speed.kmph(),
                2
            );

            Serial.print(" km/h | Satellites: ");

            Serial.print(
                gps.satellites.value()
            );

            Serial.print(" | HDOP: ");

            Serial.println(
                gps.hdop.hdop(),
                2
            );


            // =================================================
            // Check File Size
            // =================================================

            if (logFile.position() >= MAX_FILE_SIZE)
            {
                Serial.println();
                Serial.println(
                    "[SD] Maximum file size reached."
                );

                Serial.println(
                    "[SD] Rotating to a new file..."
                );

                closeCurrentFile();

                // New file will be created on the
                // next valid GPS record.
            }
        }
        else
        {
            Serial.println(
                "[GPS] Waiting for valid GPS fix..."
            );
        }
    }


    // ========================================================
    // Flush Data Every 5 Seconds
    // ========================================================

    if (logFile &&
        millis() - lastFlushTime >= FLUSH_INTERVAL)
    {
        logFile.flush();

        lastFlushTime = millis();

        Serial.println(
            "[SD] GPS data flushed to card."
        );
    }
}