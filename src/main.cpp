#include <WiFi.h>
#include <ArduinoOTA.h>

// VERSION — cambia este numero para cada release
String VERSION_LOCAL = "1.0.0";

const char* ssid     = "NAVITRACK -2.4G";
const char* password = "N@V1TR4CK";

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
}

void loop() {
    ArduinoOTA.handle();
    // tu codigo aqui
}