#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include <TinyGPS++.h>

// ============================================================
// GPS Configuration
// ============================================================

#define GPS_RX_PIN 16
#define GPS_TX_PIN 17

HardwareSerial GPS(1);

TinyGPSPlus gps;

// ============================================================
// SD Card Configuration
// ============================================================

#define SD_CS 5

const char *LOG_FILE = "/gps_data.csv";

File logFile;

// ============================================================
// Logging Configuration
// ============================================================

unsigned long lastLogTime = 0;

const unsigned long LOG_INTERVAL = 1000;   // Log every 1 second

unsigned long lastFlush = 0;

const unsigned long FLUSH_INTERVAL = 5000; // Flush every 5 seconds


// ============================================================
// SETUP
// ============================================================

void setup()
{
    Serial.begin(115200);

    delay(1000);

    Serial.println();
    Serial.println("========================================");
    Serial.println("     ESP32 GPS CSV Data Logger");
    Serial.println("========================================");

    // --------------------------------------------------------
    // Initialize GPS
    // --------------------------------------------------------

    GPS.begin(9600, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);

    Serial.println("GPS initialized.");
    Serial.println("GPS Baud Rate: 9600");

    // --------------------------------------------------------
    // Initialize SD Card
    // --------------------------------------------------------

    Serial.println();
    Serial.println("Initializing SD card...");

    if (!SD.begin(SD_CS))
    {
        Serial.println("ERROR: SD Card initialization FAILED!");
        return;
    }

    Serial.println("SD Card initialized successfully.");

    // --------------------------------------------------------
    // Display SD Card Type
    // --------------------------------------------------------

    uint8_t cardType = SD.cardType();

    Serial.print("SD Card Type: ");

    if (cardType == CARD_MMC)
        Serial.println("MMC");
    else if (cardType == CARD_SD)
        Serial.println("SDSC");
    else if (cardType == CARD_SDHC)
        Serial.println("SDHC");
    else
        Serial.println("UNKNOWN");

    // --------------------------------------------------------
    // Display SD Card Size
    // --------------------------------------------------------

    uint64_t cardSize = SD.cardSize() / (1024 * 1024);

    Serial.print("SD Card Size: ");
    Serial.print(cardSize);
    Serial.println(" MB");

    // --------------------------------------------------------
    // Open CSV File
    // --------------------------------------------------------

    logFile = SD.open(LOG_FILE, FILE_APPEND);

    if (!logFile)
    {
        Serial.println("ERROR: Could not open GPS CSV file!");
        return;
    }

    // --------------------------------------------------------
    // Create CSV Header
    // --------------------------------------------------------

    if (logFile.size() == 0)
    {
        logFile.println(
            "Date,Time,Latitude,Longitude,Speed_kmh,Satellites"
        );

        logFile.flush();

        Serial.println("CSV header created.");
    }
    else
    {
        Serial.println("Existing CSV file opened.");
    }

    Serial.println();
    Serial.print("Log File: ");
    Serial.println(LOG_FILE);

    Serial.println("GPS CSV logging started.");

    Serial.println();
    Serial.println("----------------------------------------");
}


// ============================================================
// LOOP
// ============================================================

void loop()
{
    // --------------------------------------------------------
    // Read GPS data
    // --------------------------------------------------------

    while (GPS.available())
    {
        char c = GPS.read();

        gps.encode(c);
    }

    // --------------------------------------------------------
    // Log GPS data every 1 second
    // --------------------------------------------------------

    if (millis() - lastLogTime >= LOG_INTERVAL)
    {
        lastLogTime = millis();

        // ----------------------------------------------------
        // Check if GPS has a valid position
        // ----------------------------------------------------

        if (gps.location.isValid())
        {
            // ------------------------------------------------
            // Date
            // ------------------------------------------------

            if (gps.date.isValid())
            {
                logFile.print(gps.date.day());
                logFile.print("/");
                logFile.print(gps.date.month());
                logFile.print("/");
                logFile.print(gps.date.year());
            }
            else
            {
                logFile.print("N/A");
            }

            logFile.print(",");

            // ------------------------------------------------
            // Time
            // ------------------------------------------------

            if (gps.time.isValid())
            {
                if (gps.time.hour() < 10)
                    logFile.print("0");

                logFile.print(gps.time.hour());
                logFile.print(":");

                if (gps.time.minute() < 10)
                    logFile.print("0");

                logFile.print(gps.time.minute());
                logFile.print(":");

                if (gps.time.second() < 10)
                    logFile.print("0");

                logFile.print(gps.time.second());
            }
            else
            {
                logFile.print("N/A");
            }

            logFile.print(",");

            // ------------------------------------------------
            // Latitude
            // ------------------------------------------------

            logFile.print(gps.location.lat(), 6);

            logFile.print(",");

            // ------------------------------------------------
            // Longitude
            // ------------------------------------------------

            logFile.print(gps.location.lng(), 6);

            logFile.print(",");

            // ------------------------------------------------
            // Speed
            // ------------------------------------------------

            if (gps.speed.isValid())
                logFile.print(gps.speed.kmph(), 2);
            else
                logFile.print("0.00");

            logFile.print(",");

            // ------------------------------------------------
            // Satellites
            // ------------------------------------------------

            if (gps.satellites.isValid())
                logFile.println(gps.satellites.value());
            else
                logFile.println("0");

            // ------------------------------------------------
            // Display on Serial Monitor
            // ------------------------------------------------

            Serial.print("GPS: ");

            Serial.print(gps.location.lat(), 6);
            Serial.print(", ");

            Serial.print(gps.location.lng(), 6);

            Serial.print(" | Speed: ");

            Serial.print(gps.speed.kmph(), 2);

            Serial.print(" km/h");

            Serial.print(" | Satellites: ");

            Serial.println(gps.satellites.value());
        }
        else
        {
            Serial.println("Waiting for GPS fix...");
        }
    }

    // --------------------------------------------------------
    // Flush SD card every 5 seconds
    // --------------------------------------------------------

    if (millis() - lastFlush >= FLUSH_INTERVAL)
    {
        lastFlush = millis();

        if (logFile)
        {
            logFile.flush();

            Serial.println("[SD] GPS data flushed to card.");
        }
    }
}