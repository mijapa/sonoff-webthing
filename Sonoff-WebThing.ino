#include <Ticker.h>
#include "Thing.h"
#include "WebThingAdapter.h"
#include "user_config.h"

#define LED_BUILTIN 13
#define SWITCH_PIN 12
#define KEY_PIN 0

WebThingAdapter* adapter = NULL;

const char* switchTypes[] = {"OnOffSwitch", nullptr};
ThingDevice sonoffSwitch("switch", "Sonoff RF R2 Power Switch", switchTypes);
ThingProperty switchOn("on", "Whether the switch is turned on", BOOLEAN, "OnOffProperty");

boolean isOn = false;
boolean isKeyActive = true;
Ticker keyActivator;

void activateKey(){
  isKeyActive = true;
}

void setupWiFi() {
#if defined(LED_BUILTIN)
  const int ledPin = LED_BUILTIN;
#else
  const int ledPin = 13;  // manully configure LED pin
#endif
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, HIGH);
  Serial.begin(115200);
  Serial.println("");
  Serial.print("Connecting to \"");
  Serial.print(STA_SSID1);
  Serial.println("\"");
#if defined(ESP8266) || defined(ESP32)
  WiFi.mode(WIFI_STA);
#endif
  WiFi.begin(STA_SSID1, STA_PASS1);
  Serial.println("");

  // Wait for connection
  bool blink = true;
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
    digitalWrite(ledPin, blink ? LOW : HIGH); // active low led
    blink = !blink;
  }
  digitalWrite(ledPin, HIGH); // active low led

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(STA_SSID1);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void webThingSetup() {
  adapter = new WebThingAdapter("Sonoff", WiFi.localIP());

  sonoffSwitch.addProperty(&switchOn);
  adapter->addDevice(&sonoffSwitch);
  adapter->begin();

  Serial.println("HTTP server started");
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.print("/things/");
  Serial.println(sonoffSwitch.id);
}

void setup() {
  Serial.begin(115200);
  Serial.println();
  keyActivator.attach_ms(1000, activateKey);
  setupWiFi();
  webThingSetup();
}

void loop() {
  adapter->update();
  digitalWrite(SWITCH_PIN, switchOn.getValue().boolean);
  digitalWrite(LED_BUILTIN, !switchOn.getValue().boolean);
  if (isKeyActive && !digitalRead(KEY_PIN)) {
    Serial.println("KEY");
    isOn = !isOn;
    ThingPropertyValue onValue;
    onValue.boolean = isOn;
    switchOn.setValue(onValue);
    isKeyActive = false;
  }
}
