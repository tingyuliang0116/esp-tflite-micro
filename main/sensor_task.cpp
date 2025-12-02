#include <Arduino.h>
#include "Adafruit_Sensor.h"
#include "Adafruit_AM2320.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_log.h"

static const char* TAG = "SensorSystem";

// --- Devices ---
Adafruit_AM2320 am2320 = Adafruit_AM2320();
LiquidCrystal_I2C lcd(0x27, 16, 2);

// --- Pins ---
#define TRIG_PIN 12
#define ECHO_PIN 14
#define LED_PIN  19
#define soundSpeed 0.0343

// --- Shared data ---
volatile float currentTemperature = -1;
volatile float currentHumidity = -1;
volatile float sharedDistance = -1;

SemaphoreHandle_t dataSemaphore;

// =======================================================
// TASK: Read Temperature + Humidity
// =======================================================
void temperatureHumidityTask(void *pvParams) {
    while (1) {
        float t = am2320.readTemperature();
        float h = am2320.readHumidity();

        if (!isnan(t) && !isnan(h)) {
            xSemaphoreTake(dataSemaphore, portMAX_DELAY);
            currentTemperature = t;
            currentHumidity = h;
            xSemaphoreGive(dataSemaphore);

            ESP_LOGI(TAG, "Temp: %.1f°C Hum: %.0f%%", t, h);
        } else {
            ESP_LOGW(TAG, "AM2320 read failed");
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

// =======================================================
// TASK: Ultrasonic Sensor
// =======================================================
void ultrasonicTask(void *pvParams) {
    while (1) {
        digitalWrite(TRIG_PIN, LOW);
        delayMicroseconds(10);
        digitalWrite(TRIG_PIN, HIGH);
        delayMicroseconds(10);
        digitalWrite(TRIG_PIN, LOW);

        long duration = pulseIn(ECHO_PIN, HIGH, 30000);
        float d = (duration * soundSpeed) / 2.0;

        xSemaphoreTake(dataSemaphore, portMAX_DELAY);
        sharedDistance = d;
        xSemaphoreGive(dataSemaphore);

        ESP_LOGI(TAG, "Distance: %.1f cm", d);

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

// =======================================================
// TASK: LCD Display
// =======================================================
void LCDTask(void *arg) {
    lcd.init();
    lcd.backlight();

    float lastT = -100, lastH = -100, lastD = -100;

    while (1) {
        xSemaphoreTake(dataSemaphore, portMAX_DELAY);
        float t = currentTemperature;
        float h = currentHumidity;
        float d = sharedDistance;
        xSemaphoreGive(dataSemaphore);

        if (t != lastT || h != lastH || d != lastD) {
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.printf("T: %.1f H: %.0f%%", t, h);

            lcd.setCursor(0, 1);
            lcd.printf("Dist: %.1fcm", d);

            lastT = t;
            lastH = h;
            lastD = d;
        }

        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

// =======================================================
// TASK: Alarm (LED Blink)
// =======================================================
void alarmTask(void *arg) {
    while (1) {
        xSemaphoreTake(dataSemaphore, portMAX_DELAY);
        float t = currentTemperature;
        float h = currentHumidity;
        float d = sharedDistance;
        xSemaphoreGive(dataSemaphore);

        bool distanceAlert = (d > 10 && d < 120);
        bool tempAlert     = (t > 30);
        bool humidityAlert = (h > 75);

        if (distanceAlert || tempAlert || humidityAlert) {
            ESP_LOGW(TAG, "ALARM TRIGGERED!");
            for (int i = 0; i < 3; i++) {
                digitalWrite(LED_PIN, HIGH);
                vTaskDelay(pdMS_TO_TICKS(200));
                digitalWrite(LED_PIN, LOW);
                vTaskDelay(pdMS_TO_TICKS(200));
            }
        } else {
            digitalWrite(LED_PIN, LOW);
        }

        vTaskDelay(pdMS_TO_TICKS(150));
    }
}

// =======================================================
// ENTRY POINT CALLED FROM main.cc
// =======================================================
extern "C" void sensorTask(void *pvParameter) {
    ESP_LOGI(TAG, "SensorTask started on core %d", xPortGetCoreID());

    // Init hardware
    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);

    am2320.begin();
    Wire.begin();

    // Create semaphore
    dataSemaphore = xSemaphoreCreateMutex();
    xSemaphoreGive(dataSemaphore);

    // Start tasks
    xTaskCreatePinnedToCore(temperatureHumidityTask, "TempHum", 4096, NULL, 2, NULL, 1);
    xTaskCreatePinnedToCore(ultrasonicTask,        "Ultrasonic", 4096, NULL, 2, NULL, 1);
    xTaskCreatePinnedToCore(LCDTask,               "LCD", 4096, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(alarmTask,             "Alarm", 4096, NULL, 1, NULL, 1);

    // Kill parent stub task
    vTaskDelete(NULL);
}