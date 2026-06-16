#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Update.h>

#define VERSION_URL "https://raw.githubusercontent.com/jhonoresulca/esp32-ota-cicd/main/version.txt"
#define FIRMWARE_URL "https://github.com/jhonoresulca/esp32-ota-cicd/releases/latest/download/firmware.bin"
// VERSION — cambia este numero para cada release
String VERSION_LOCAL = "1.0.3";

unsigned long lastCheck = 0;

void checkAndUpdate() {
    Serial.println("[GitHub] Consultando versión...");
    
    HTTPClient http;
    //http.setInsecure();  // <- sin esto falla GitHub HTTPS
    http.begin(VERSION_URL);
    int code = http.GET();
    
    Serial.print("[DEBUG] HTTP code: ");
    Serial.println(code);
    
    if (code == 200) {
        String newVersion = http.getString();
        newVersion.trim();
        Serial.print("GitHub versión: ");
        Serial.println(newVersion);
        
        if (newVersion != VERSION_LOCAL) {
            Serial.println("✓ Descargando firmware...");
            http.end();
            delay(500);
            
            Serial.println("[DEBUG] Iniciando descarga...");
            http.begin(FIRMWARE_URL);
            int httpCode = http.GET();
            
            Serial.print("[DEBUG] HTTP code descarga: ");
            Serial.println(httpCode);
            
            if (httpCode == 200) {
                int len = http.getSize();
                Serial.print("Tamaño: ");
                Serial.println(len);
                
                if (Update.begin(len)) {
                    WiFiClient* stream = http.getStreamPtr();
                    size_t written = Update.writeStream(*stream);
                    
                    Serial.print("Bytes escritos: ");
                    Serial.println(written);
                    
                    if (Update.end()) {
                        Serial.println("✓ ¡Actualizado!");
                        delay(1000);
                        ESP.restart();
                    } else {
                        Serial.print("✗ Update error: ");
                        Serial.println(Update.getError());
                    }
                } else {
                    Serial.println("✗ No se pudo iniciar Update");
                }
            } else {
                Serial.print("✗ HTTP error: ");
                Serial.println(httpCode);
            }
        } else {
            Serial.println("✓ Versión actual");
        }
    } else {
        Serial.print("✗ Fallo conexión: ");
        Serial.println(code);
    }
    http.end();
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    pinMode(2, OUTPUT);
    
    Serial.println("\n=== OTA desde GitHub ===");
    
    WiFi.begin("NAVITRACK -2.4G", "N@V1TR4CK");
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    
    Serial.print("\nIP: ");
    Serial.println(WiFi.localIP());
}

void loop() {
    // LED
    digitalWrite(2, HIGH);
    Serial.println("LED ON");
    delay(1000);
    
    digitalWrite(2, LOW);
    Serial.println("LED OFF");
    delay(1000);
    
    // Consultar cada 30 segundos
    if (millis() - lastCheck > 10000) {
        lastCheck = millis();
        checkAndUpdate();
    }
}