//ardiuno Can catapult Reciever code - V3

#include <WiFi.h>
#include <esp_now.h>

#define LED_PIN 2          // blink LED
#define PAUSE_LED_PIN 15   // lights while the sender is paused

// Must match the sender exactly
typedef struct {
  int count;
  bool ledState;
  bool paused;
} Message;

Message msg;

// Runs automatically whenever a message arrives
// (on older ESP32 core 2.x, change the first parameter to: const uint8_t *mac)
void onReceive(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
  memcpy(&msg, data, sizeof(msg));
  digitalWrite(LED_PIN, msg.ledState);
  digitalWrite(PAUSE_LED_PIN, msg.paused);
  Serial.printf("Received #%d, LED=%d, paused=%d\n", msg.count, msg.ledState, msg.paused);
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  pinMode(PAUSE_LED_PIN, OUTPUT);
  digitalWrite(PAUSE_LED_PIN, LOW);
  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed");
    return;
  }
  esp_now_register_recv_cb(onReceive);
}

void loop() {
  // nothing needed: the callback does the work
}