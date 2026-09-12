#include <Arduino.h>
#include <SPI.h>
#include <SD.h>

// ============================================================
// GPS Configuration
// ============================================================

#define GPS_RX_PIN 16
#define GPS_TX_PIN 17

HardwareSerial GPS(1);

// ============================================================
// SD Card Configuration
// ============================================================

#define SD_CS 5

// GPS log file
const char *LOG_FILE = "/gps_log.txt";

// ============================================================
// Variables
// ============================================================

File logFile;

String nmeaSentence = "";

unsigned long lastFlush = 0;

const unsigned long FLUSH_INTERVAL = 5000;   // 5 seconds

// ============================================================
// Setup
// ============================================================

void setup()
{
    Serial.begin(115200);

    delay(1000);

    Serial.println();
    Serial.println("========================================");
    Serial.println("     ESP32 GPS SD Card Data Logger");
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
    // Check SD Card Type
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
    // Open GPS Log File
    // --------------------------------------------------------

    logFile = SD.open(LOG_FILE, FILE_APPEND);

    if (!logFile)
    {
        Serial.println("ERROR: Could not open GPS log file!");
        return;
    }

    // --------------------------------------------------------
    // Write Session Header
    // --------------------------------------------------------

    logFile.println();
    logFile.println("========================================");
    logFile.println("New GPS Logging Session");
    logFile.println("========================================");

    logFile.flush();

    Serial.println();
    Serial.print("Log File: ");
    Serial.println(LOG_FILE);

    Serial.println("GPS logging started.");
    Serial.println();
    Serial.println("----------------------------------------");
}

// ============================================================
// Loop
// ============================================================

void loop()
{
    // --------------------------------------------------------
    // Read GPS data
    // --------------------------------------------------------

    while (GPS.available())
    {
        char c = GPS.read();

        // Display raw GPS data on Serial Monitor
        Serial.write(c);

        // ----------------------------------------------------
        // Build complete NMEA sentence
        // ----------------------------------------------------

        if (c == '\n')
        {
            // Complete NMEA sentence received

            if (nmeaSentence.length() > 0)
            {
                // Write complete sentence to SD card
                logFile.println(nmeaSentence);

                // Clear buffer
                nmeaSentence = "";
            }
        }
        else if (c != '\r')
        {
            // Add character to sentence
            nmeaSentence += c;
        }

        // ----------------------------------------------------
        // Prevent String from growing indefinitely
        // ----------------------------------------------------

        if (nmeaSentence.length() > 120)
        {
            nmeaSentence = "";
        }
    }

    // --------------------------------------------------------
    // Flush SD card periodically
    // --------------------------------------------------------

    if (millis() - lastFlush >= FLUSH_INTERVAL)
    {
        logFile.flush();

        lastFlush = millis();

        Serial.println();
        Serial.println("[SD] Data flushed to card.");
    }
}