// Basic demo for accelerometer readings from Adafruit MPU6050

// ESP32 Guide: https://RandomNerdTutorials.com/esp32-mpu-6050-accelerometer-gyroscope-arduino/
// ESP8266 Guide: https://RandomNerdTutorials.com/esp8266-nodemcu-mpu-6050-accelerometer-gyroscope-arduino/
// Arduino Guide: https://RandomNerdTutorials.com/arduino-mpu-6050-accelerometer-gyroscope/
#include "Arduino.h"
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include "WiFi.h"
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

Adafruit_MPU6050 mpu;
bool doLog = false;

AsyncWebServer server(80);

void setupMPU() {
    Serial.println("Adafruit MPU6050 test!");



    // Try to initialize!
    if (!mpu.begin()) {
        Serial.println("Failed to find MPU6050 chip");
        while (1) {
            delay(10);
        }
    }
    Serial.println("MPU6050 Found!");

    mpu.setAccelerometerRange(MPU6050_RANGE_16_G);
    Serial.print("Accelerometer range set to: ");
    switch (mpu.getAccelerometerRange()) {
        case MPU6050_RANGE_2_G:
            Serial.println("+-2G");
            break;
        case MPU6050_RANGE_4_G:
            Serial.println("+-4G");
            break;
        case MPU6050_RANGE_8_G:
            Serial.println("+-8G");
            break;
        case MPU6050_RANGE_16_G:
            Serial.println("+-16G");
            break;
    }
    mpu.setGyroRange(MPU6050_RANGE_2000_DEG);
    Serial.print("Gyro range set to: ");
    switch (mpu.getGyroRange()) {
        case MPU6050_RANGE_250_DEG:
            Serial.println("+- 250 deg/s");
            break;
        case MPU6050_RANGE_500_DEG:
            Serial.println("+- 500 deg/s");
            break;
        case MPU6050_RANGE_1000_DEG:
            Serial.println("+- 1000 deg/s");
            break;
        case MPU6050_RANGE_2000_DEG:
            Serial.println("+- 2000 deg/s");
            break;
    }

    mpu.setFilterBandwidth(MPU6050_BAND_260_HZ);
    Serial.print("Filter bandwidth set to: ");
    switch (mpu.getFilterBandwidth()) {
        case MPU6050_BAND_260_HZ:
            Serial.println("260 Hz");
            break;
        case MPU6050_BAND_184_HZ:
            Serial.println("184 Hz");
            break;
        case MPU6050_BAND_94_HZ:
            Serial.println("94 Hz");
            break;
        case MPU6050_BAND_44_HZ:
            Serial.println("44 Hz");
            break;
        case MPU6050_BAND_21_HZ:
            Serial.println("21 Hz");
            break;
        case MPU6050_BAND_10_HZ:
            Serial.println("10 Hz");
            break;
        case MPU6050_BAND_5_HZ:
            Serial.println("5 Hz");
            break;
    }

    Serial.println("");
    delay(100);
}

void setupSD() {
    if (!SD.begin(5)) {
        Serial.println("Card Mount Failed");
        ESP.restart();
        return;
    }
    uint8_t cardType = SD.cardType();

    if (cardType == CARD_NONE) {
        Serial.println("No SD card attached");
        return;
    }

    Serial.print("SD Card Type: ");
    if (cardType == CARD_MMC) {
        Serial.println("MMC");
    } else if (cardType == CARD_SD) {
        Serial.println("SDSC");
    } else if (cardType == CARD_SDHC) {
        Serial.println("SDHC");
    } else {
        Serial.println("UNKNOWN");
        ESP.restart();
    }

    uint64_t cardSize = SD.cardSize() / (1024 * 1024);
    Serial.printf("SD Card Size: %lluMB\n", cardSize);
}

void setup(void) {
    Serial.begin(115200);

    pinMode(15, OUTPUT);
    digitalWrite(15, LOW);

    setupSD();

    IPAddress localIP(192,168,1,2);
    IPAddress gateway(192,168,1,1);
    IPAddress subnet(255,255,255,0);

    //WiFi.softAPConfig(localIP, gateway, subnet);
    WiFi.softAP("Flight Recorder", "123456789");

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(SD, "/index.html", "text/html");
    });
    server.on("/startLog", HTTP_POST, [](AsyncWebServerRequest *request) {
        doLog = true;
        request->send(200, "text/plain", "OK");
    });

    server.serveStatic("/", SD, "/");

    server.begin();

    setupMPU();

    delay(3000);

    digitalWrite(15, HIGH);
    delay(5000);
    digitalWrite(15, LOW);

    Serial.println("Started");
}

void loop() {
    while (!doLog) {
        delay(100);
    }

    doLog = false;
    delay(100);
    server.end();

    delay(10);

    int numFiles = 0;
    File testRoot = SD.open("/tests");
    while (true) {
        File entry = testRoot.openNextFile();
        if (!entry) {
            // no more files
            break;
        }
        numFiles++;
        entry.close();
    }
    testRoot.close();

    Serial.print("Files on SD card: ");
    Serial.println(numFiles);

    delay(100);
    String numFilesStr = String(numFiles+1);
    File file = SD.open("/tests/test" + numFilesStr + ".txt", FILE_APPEND);
    if (!file) {
        Serial.println("Failed to open file for writing");
    }

    int i = 0;

    /*unsigned long totX = 0;
    unsigned long totY = 0;
    unsigned long totZ = 0;
    unsigned long totRX = 0;
    unsigned long totRY = 0;
    unsigned long totRZ = 0;*/

    unsigned long microsStart = micros();
    int timeWithNoMovement = 0;
    unsigned long millisStart = millis();
    bool logRunning = true;
    while (i < 260 * 10 * 60 && logRunning) {
        i++;
        sensors_event_t a, g, temp;
        mpu.getEvent(&a, &g, &temp);

        /*totX += abs(a.acceleration.x);
        totY += abs(a.acceleration.y);
        totZ += abs(a.acceleration.z);

        totRX += abs(g.gyro.x);
        totRY += abs(g.gyro.y);
        totRZ += abs(g.gyro.z);*/


        file.print(micros() - microsStart);
        file.print(", ");
        file.print(a.acceleration.x);
        file.print(", ");
        file.print(a.acceleration.y);
        file.print(", ");
        file.print(a.acceleration.z);
        file.print(", ");
        file.print(g.gyro.x);
        file.print(", ");
        file.print(g.gyro.y);
        file.print(", ");
        file.print(g.gyro.z);
        file.print(", ");
        file.println(temp.temperature);

        if (i%20 == 0) {
            Serial.println(timeWithNoMovement);
            Serial.println(i);
            Serial.println(pow(a.acceleration.x, 2) + pow(a.acceleration.y, 2) + pow(a.acceleration.z, 2));
        }

        if (i >= 260 * 60) {
            if (pow(a.acceleration.x, 2) + pow(a.acceleration.y, 2) + pow(a.acceleration.z, 2) <= pow(12, 2) &&
            pow(g.gyro.x, 2) + pow(g.gyro.y, 2) + pow(g.gyro.z, 2) <= 1) {
                timeWithNoMovement++;
                if (timeWithNoMovement >= 260 * 60) {
                    logRunning = false;
                }
            } else {
                timeWithNoMovement -= 10;
                timeWithNoMovement = max(timeWithNoMovement, 0);
            }
        }

        unsigned long nextUs = microsStart + (i+1)*1000.0/260.0 * 1000.0;
        unsigned long delay = nextUs-micros();
        if (delay <= 1000*1000/260) {
            delay = 0;
        }
        //Serial.printf("Delaying %lu ms, millis = %lu, start = %lu, next = %lu\n", nextUs-micros(), micros(), microsStart, nextUs);
        //delayMicroseconds(delay);
        //delay(1000 / 260);
    }
    unsigned long millisEnd = millis();

    file.close();
    Serial.println("Log done");
    Serial.printf("Logged for %lu seconds\n", (millisEnd-millisStart)/1000);

    digitalWrite(15, HIGH);
}