//ardiuno Can catapult sender code - V3

//confirmation LED
#define LED_PIN 2

//pause indicator LED on the sender (lights while paused)
#define PAUSE_LED_PIN 15

//pause button
#define BUTTON_PIN 4
#define PAUSE_TIME 3000       // pause length in ms (3 seconds)
#define SEND_INTERVAL 1000    // time between messages in ms

//wifi setup
#include <WiFi.h>
#include <esp_now.h>

// Broadcast address: any ESP32 in range will receive the message
uint8_t peerAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// The data packet both boards agree on (must match the receiver)
typedef struct {
  int count;
  bool ledState;
  bool paused;       // NEW: tells the receiver whether we are paused
} Message;

Message msg = {0, false, false};

// Pause and timing variables
bool paused = false;
unsigned long pauseStart = 0;
unsigned long lastSend = 0;
bool lastButton = HIGH;       // HIGH = not pressed (pull-up)

// Fills in the pause flag and transmits the current message
void sendMessage() {
  msg.count++;
  msg.paused = paused;
  esp_err_t result = esp_now_send(peerAddress, (uint8_t *)&msg, sizeof(msg));
  Serial.printf("Sent #%d, LED=%d, paused=%d, %s\n", msg.count, msg.ledState,
                msg.paused, result == ESP_OK ? "OK" : "ERROR");
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  pinMode(PAUSE_LED_PIN, OUTPUT);
  digitalWrite(PAUSE_LED_PIN, LOW);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  WiFi.mode(WIFI_STA);                 // ESP-NOW needs Wi-Fi radio on

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed");
    return;
  }

  esp_now_peer_info_t peer = {};
  memcpy(peer.peer_addr, peerAddress, 6);
  peer.channel = 0;
  peer.encrypt = false;

  if (esp_now_add_peer(&peer) != ESP_OK) {
    Serial.println("Failed to add peer");
  }
}

void loop() {
  // 1. Detect the moment the button is pressed
  bool button = digitalRead(BUTTON_PIN);
  if (lastButton == HIGH && button == LOW) {
    paused = true;
    pauseStart = millis();
    digitalWrite(PAUSE_LED_PIN, HIGH);   // sender pause LED ON
    Serial.println("Button pressed: paused for 3 seconds");
    sendMessage();                       // tell the receiver we are paused
    delay(30);                           // ignore button bounce
  }
  lastButton = button;

  // 2. End the pause after 3 seconds
  if (paused && millis() - pauseStart >= PAUSE_TIME) {
    paused = false;
    digitalWrite(PAUSE_LED_PIN, LOW);    // sender pause LED OFF
    Serial.println("Resumed");
    sendMessage();                       // tell the receiver we resumed
    lastSend = millis();
  }

  // 3. Send the next blink message unless paused
  if (!paused && millis() - lastSend >= SEND_INTERVAL) {
    lastSend = millis();
    msg.ledState = !msg.ledState;            // flip ON/OFF each time
    digitalWrite(LED_PIN, msg.ledState);     // local LED matches what was sent
    sendMessage();
  }
}
