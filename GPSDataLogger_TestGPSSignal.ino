#include <TinyGPS++.h>

TinyGPSPlus gps;

HardwareSerial gpsSerial(2);

#define GPS_RX 16
#define GPS_TX 17

void setup() {
    delay(2000);

  Serial.begin(115200);

  Serial.println();
  Serial.println("ESP32 GPS Test");
  Serial.println("----------------");

  gpsSerial.begin(
    9600,
    SERIAL_8N1,
    GPS_RX,
    GPS_TX
  );

  Serial.println("GPS Serial Started");
}

void loop() {

  while (gpsSerial.available()) {

    char c = gpsSerial.read();
    Serial.write(c);
    gps.encode(c);
    
  }
  if (gps.location.isUpdated()) {

    Serial.println("----------------");

    Serial.print("Latitude: ");
    Serial.println(gps.location.lat(), 6);

    Serial.print("Longitude: ");
    Serial.println(gps.location.lng(), 6);

    Serial.print("Speed (km/h): ");
    Serial.println(gps.speed.kmph());

    Serial.print("Altitude (m): ");
    Serial.println(gps.altitude.meters());

    Serial.print("Satellites: ");
    Serial.println(gps.satellites.value());

    Serial.print("HDOP: ");
    Serial.println(gps.hdop.hdop());

    if (gps.time.isValid()) {

      Serial.print("UTC Time: ");

      if (gps.time.hour() < 10)
        Serial.print("0");

      Serial.print(gps.time.hour());
      Serial.print(":");

      if (gps.time.minute() < 10)
        Serial.print("0");

      Serial.print(gps.time.minute());
      Serial.print(":");

      if (gps.time.second() < 10)
        Serial.print("0");

      Serial.println(gps.time.second());
    }
  }
}