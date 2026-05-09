/**
 * Ball Sensor ESP32
 * Detects ball entry on left or right sensor and sends an ESP-NOW
 * trigger to the cateye ESP32 to start the celebration display.
 */

#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>


#define SENSOR_LEFT  15
#define SENSOR_RIGHT 12

#define DEBOUNCE_MS 200

static uint8_t cateyeMac[] = {0xA0, 0xDD, 0x6C, 0x74, 0xD3, 0xB4};

static bool          lastLeft  = false;
static bool          lastRight = false;
static unsigned long lastTriggerLeft  = 0;
static unsigned long lastTriggerRight = 0;

void sendCelebration() {
    uint8_t msg = 1;
    esp_err_t result = esp_now_send(cateyeMac, &msg, sizeof(msg));
    Serial.printf("[ESP-NOW] Trigger sent: %s\n", result == ESP_OK ? "OK" : "FAILED");
}

void setup() {
    Serial.begin(115200);

    pinMode(SENSOR_LEFT,  INPUT_PULLDOWN);
    pinMode(SENSOR_RIGHT, INPUT_PULLDOWN);

    WiFi.mode(WIFI_STA);
    WiFi.disconnect();

    if (esp_now_init() != ESP_OK) {
        Serial.println("[ESP-NOW] Init FAILED");
        return;
    }

    esp_now_peer_info_t peer = {};
    memcpy(peer.peer_addr, cateyeMac, 6);
    peer.channel = 0;
    peer.encrypt = false;
    esp_now_add_peer(&peer);

    Serial.println("[BALL SENSOR] Ready — waiting for ball detections...");
}

void loop() {
    unsigned long now = millis();

    bool left  = (digitalRead(SENSOR_LEFT)  == HIGH);
    bool right = (digitalRead(SENSOR_RIGHT) == HIGH);

    if (left && !lastLeft && (now - lastTriggerLeft >= DEBOUNCE_MS)) {
        lastTriggerLeft = now;
        Serial.println("[BALL] LEFT sensor triggered!");
        sendCelebration();
    }

    if (right && !lastRight && (now - lastTriggerRight >= DEBOUNCE_MS)) {
        lastTriggerRight = now;
        Serial.println("[BALL] RIGHT sensor triggered!");
        sendCelebration();
    }

    lastLeft  = left;
    lastRight = right;

    delay(10);
}
