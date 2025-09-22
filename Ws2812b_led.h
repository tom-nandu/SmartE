#pragma once

#include <FastLED.h>
#include <IIOT_V4_Pinout.h>
#define NUM_LEDS 1
// Define the array of leds
xQueueHandle queueHandleIdWsLed;

// extern CRGB leds[NUM_LEDS];
#define STATUS_LED (leds[0])
#define NETWORK_LED (leds[1])
class ws2812b_led {
   private:
    /* data */
    // CRGB STATUS_LED =(leds[0]);
    // CRGB NETWORK_LED =(leds[0]);

   public:
    ws2812b_led(/* args */);
    ~ws2812b_led();
    void init();
    // void thread();
};

ws2812b_led::ws2812b_led(/* args */) {
}

ws2812b_led::~ws2812b_led() {
}

void wsledTask(void* pvParameters) {
    CRGB leds[NUM_LEDS];
    FastLED.addLeds<WS2812B, PIN_LED_WS2812_DATA, GRB>(leds, NUM_LEDS);  // GRB ordering is typical
    FastLED.setBrightness(10);
    STATUS_LED = CRGB::Green;
    FastLED.show();
    static CRGB ledColor;
    TaskHandle_t taskHandle = xTaskGetCurrentTaskHandle();
    long _millis_last = millis();
    while (1) {
        /* code */
        if (xQueueReceive(queueHandleIdWsLed, &ledColor, 0) == pdPASS) {
            STATUS_LED = ledColor;
        }
        if (millis() - _millis_last > 2000) {
            // query the remaining free memory in the reserved area,
            //  The smaller the returned number the closer the task has come to overflowing its stack.
            log_v("TASK: %s : %d ", pcTaskGetName(taskHandle), uxTaskGetStackHighWaterMark(taskHandle));
            _millis_last = millis();
        }
        FastLED.show();
        delay(100);
    }
}


void ws2812b_led::init() {
    //    const uint8_t _p = _pinNumber;
    queueHandleIdWsLed = xQueueCreate(1, sizeof(CRGB));
    xTaskCreate(
        wsledTask,
        "wsledTask",  // A name just for humans
        1024*3,         // The stack size
        NULL,         // Task parameter
        1,            // Priority
        NULL          // Task handle is not used here - simply pass NULL
    );

    //   STATUS_LED = CRGB::Red;   // App not started
    //   NETWORK_LED = CRGB::Red;  // Network not connected
}
