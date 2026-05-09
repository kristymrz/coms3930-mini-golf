#include <Arduino.h>
#include <TFT_eSPI.h>
#include <esp_now.h>
#include <WiFi.h>

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite spr = TFT_eSprite(&tft);

const int SCR_W = 240;
const int SCR_H = 135;

#define CELEBRATE_DURATION_MS 13700

volatile bool celebrateFlag = false;
bool celebrating = false;
unsigned long celebrateStartMs = 0;

enum ScrollPhase { SLIDE_IN, HOLD, SLIDE_OUT };

ScrollPhase idlePhase = SLIDE_IN;
int idleOffset = SCR_W;           // goes SCR_W → centeredX → exits left
unsigned long idleHoldStart = 0;
#define IDLE_HOLD_MS  2000
#define SCROLL_SPEED  4

// celebration offset: SCR_W → 0 (centered) → -SCR_W (fully off left)
ScrollPhase celebPhase = SLIDE_IN;
int celebOffset = SCR_W;
unsigned long celebHoldStart = 0;
#define CELEB_HOLD_MS 3000

const uint16_t COL_SKY = TFT_SKYBLUE;

void onDataReceived(const uint8_t* mac, const uint8_t* data, int len) {
    if (len > 0 && data[0] == 1 && !celebrating) {
        celebrateFlag = true;
    }
}

// outlined text using drawString (no wrapping)
void sprOutlined(const char* text, int x, int y, uint16_t fg) {
    spr.setTextDatum(TL_DATUM);
    spr.setTextColor(TFT_BLACK);
    for (int dx = -1; dx <= 1; dx++) {
        for (int dy = -1; dy <= 1; dy++) {
            if (dx != 0 || dy != 0) {
                spr.drawString(text, x + dx, y + dy);
            }
        }
    }
    spr.setTextColor(fg);
    spr.drawString(text, x, y);
}

void renderIdle() {
    spr.fillSprite(COL_SKY);
    spr.setTextSize(4);
    // "MINI-GOLF": 9 chars * 6px * size4 = 216px wide
    sprOutlined("MINI-GOLF", idleOffset, SCR_H / 2 - 16, TFT_WHITE);
    spr.pushSprite(0, 0);
}

void renderCelebration() {
    uint16_t bg = tft.color565(10, 0, 40);
    spr.fillSprite(bg);

    // rainbow border (always fixed, doesn't slide)
    for (int i = 0; i < 4; i++) {
        spr.drawRect(i, i, SCR_W - i * 2, SCR_H - i * 2,
                     i == 0 ? TFT_RED :
                     i == 1 ? TFT_YELLOW :
                     i == 2 ? TFT_GREEN : TFT_CYAN);
    }

    spr.setTextSize(2);

    // each line centered independently + shifted by celebOffset
    // textSize 2: each char = 12px wide, height ~16px
    // "CONGRATULATIONS!!!!" = 20 * 12 = 240px
    int cx1 = (SCR_W - 240) / 2 + celebOffset;   // x = celebOffset
    // "YOU GOT IT IN" = 13 * 12 = 156px
    int cx2 = (SCR_W - 156) / 2 + celebOffset;   // x = 42 + celebOffset
    // "THE LIGHTHOUSE!" = 15 * 12 = 180px
    int cx3 = (SCR_W - 180) / 2 + celebOffset;   // x = 30 + celebOffset

    spr.setTextDatum(TL_DATUM);

    spr.setTextColor(tft.color565(255, 0, 255));
    spr.drawString("CONGRATULATIONS!!!!", cx1, 18);

    spr.drawFastHLine(10, 46, SCR_W - 20, TFT_YELLOW);

    spr.setTextColor(TFT_CYAN);
    spr.drawString("YOU GOT IT IN", cx2, 56);

    spr.setTextColor(tft.color565(255, 140, 0));
    spr.drawString("THE LIGHTHOUSE!", cx3, 84);

    // corner dots
    spr.fillCircle(20,  20,  4, TFT_YELLOW);
    spr.fillCircle(220, 20,  4, TFT_GREEN);
    spr.fillCircle(20,  115, 4, TFT_RED);
    spr.fillCircle(220, 115, 4, TFT_MAGENTA);

    spr.pushSprite(0, 0);
}

void setup() {
    Serial.begin(115200);

    tft.init();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);

    spr.setColorDepth(16);
    if (!spr.createSprite(SCR_W, SCR_H)) {
        Serial.println("[SPRITE] Allocation FAILED");
    } else {
        Serial.println("[SPRITE] Allocated OK");
    }

    WiFi.mode(WIFI_STA);
    WiFi.disconnect();

    if (esp_now_init() != ESP_OK) {
        Serial.println("[ESP-NOW] Init FAILED");
    } else {
        esp_now_register_recv_cb(onDataReceived);
        Serial.println("[ESP-NOW] Ready");
    }

    renderIdle();
}

void loop() {
    unsigned long now = millis();

    if (celebrateFlag && !celebrating) {
        celebrateFlag = false;
        celebrating = true;
        celebrateStartMs = now;
        celebPhase = SLIDE_IN;
        celebOffset = SCR_W;
        Serial.println("[CATEYE] Celebrating!");
    }

    if (celebrating) {
        if (now - celebrateStartMs >= CELEBRATE_DURATION_MS) {
            celebrating = false;
            idleOffset = SCR_W;
            idlePhase = SLIDE_IN;
            renderIdle();
            return;
        }

        switch (celebPhase) {
            case SLIDE_IN:
                celebOffset -= SCROLL_SPEED;
                if (celebOffset <= 0) {
                    celebOffset = 0;
                    celebPhase = HOLD;
                    celebHoldStart = now;
                }
                break;
            case HOLD:
                if (now - celebHoldStart >= CELEB_HOLD_MS) {
                    celebPhase = SLIDE_OUT;
                }
                break;
            case SLIDE_OUT:
                celebOffset -= SCROLL_SPEED;
                if (celebOffset <= -SCR_W) {
                    celebOffset = SCR_W;
                    celebPhase = SLIDE_IN;
                }
                break;
        }

        renderCelebration();
        delay(20);
        return;
    }

    // idle: "MINI-GOLF" slides in to center, holds, slides out left
    const int TEXT_W    = 216;
    const int centeredX = (SCR_W - TEXT_W) / 2;

    switch (idlePhase) {
        case SLIDE_IN:
            idleOffset -= SCROLL_SPEED;
            if (idleOffset <= centeredX) {
                idleOffset = centeredX;
                idlePhase = HOLD;
                idleHoldStart = now;
            }
            break;
        case HOLD:
            if (now - idleHoldStart >= IDLE_HOLD_MS) {
                idlePhase = SLIDE_OUT;
            }
            break;
        case SLIDE_OUT:
            idleOffset -= SCROLL_SPEED;
            if (idleOffset < -TEXT_W) {
                idleOffset = SCR_W;
                idlePhase = SLIDE_IN;
            }
            break;
    }

    renderIdle();
    delay(20);
}
