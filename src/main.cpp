#include <WiFi.h>
#include <ArduinoOTA.h>

// VERSION — cambia este numero para cada release
String VERSION_LOCAL = "1.0.2";

const char* ssid     = "NAVITRACK -2.4G";
const char* password = "N@V1TR4CK";
#define LED 48
void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("Firmware version: " + VERSION_LOCAL);

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500); Serial.print(".");
    }
    Serial.println("\nIP: " + WiFi.localIP().toString());

    ArduinoOTA.begin();
    Serial.println("OTA listo");
    /************************************************** */

    pinMode(LED,OUTPUT);
}

void loop() {
    ArduinoOTA.handle();
    digitalWrite(LED,1);
    delay(100);
    digitalWrite(LED,0);
    delay(100);
    
    // tu codigo aqui
}