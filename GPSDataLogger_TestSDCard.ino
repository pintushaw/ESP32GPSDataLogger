#include <SPI.h>
#include <SD.h>

#define SD_CS 5   // Change this if your SD module uses another CS pin

void setup()
{
    Serial.begin(115200);
    delay(1000);

    Serial.println();
    Serial.println("=== ESP32 SD Card Test ===");

    // Initialize SD card
    if (!SD.begin(SD_CS))
    {
        Serial.println("SD Card initialization FAILED!");
        return;
    }

    Serial.println("SD Card initialized successfully.");

    // Check card type
    uint8_t cardType = SD.cardType();

    if (cardType == CARD_NONE)
    {
        Serial.println("No SD card detected.");
        return;
    }

    Serial.print("SD Card Type: ");

    if (cardType == CARD_MMC)
        Serial.println("MMC");
    else if (cardType == CARD_SD)
        Serial.println("SDSC");
    else if (cardType == CARD_SDHC)
        Serial.println("SDHC");
    else
        Serial.println("UNKNOWN");

    // Card size
    uint64_t cardSize = SD.cardSize() / (1024 * 1024);

    Serial.print("SD Card Size: ");
    Serial.print(cardSize);
    Serial.println(" MB");

    // Write test
    Serial.println();
    Serial.println("Writing test file...");

    File file = SD.open("/test.txt", FILE_WRITE);

    if (!file)
    {
        Serial.println("Failed to open test.txt for writing.");
        return;
    }

    file.println("ESP32 SD Card Test");
    file.println("SD card write successful.");

    file.close();

    Serial.println("Write successful.");

    // Read test
    Serial.println();
    Serial.println("Reading test file...");

    file = SD.open("/test.txt");

    if (!file)
    {
        Serial.println("Failed to open test.txt for reading.");
        return;
    }

    while (file.available())
    {
        Serial.write(file.read());
    }

    file.close();

    Serial.println();
    Serial.println("Read successful.");

    Serial.println();
    Serial.println("=== SD Card Test Completed ===");
}

void loop()
{
}