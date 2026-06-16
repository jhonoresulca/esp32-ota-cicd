#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>  // necesario para HTTPS
#include <Update.h>

// URLs del repositorio GitHub
#define VERSION_URL  "https://raw.githubusercontent.com/jhonoresulca/esp32-ota-cicd/main/version.txt"
#define FIRMWARE_URL "https://github.com/jhonoresulca/esp32-ota-cicd/releases/latest/download/firmware.bin"

// Versión actual del firmware — cambiar en cada release
String VERSION_LOCAL = "1.0.5";

unsigned long lastCheck = 0;

void checkAndUpdate() {
    Serial.println("[GitHub] Consultando versión...");

    // Cliente seguro para HTTPS — setInsecure() omite verificación de certificado
    WiFiClientSecure client;
    client.setInsecure();

    HTTPClient http;
    // Necesario para GitHub releases que devuelve redirect 302
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);

    // ── PASO 1: Consultar versión disponible en GitHub ──────────────────
    http.begin(client, VERSION_URL);
    int code = http.GET();

    Serial.print("[DEBUG] HTTP code versión: ");
    Serial.println(code);

    if (code != 200) {
        Serial.print("✗ Fallo al consultar versión: ");
        Serial.println(code);
        http.end();
        return;
    }

    String newVersion = http.getString();
    newVersion.trim();  // elimina saltos de línea del archivo
    Serial.print("GitHub versión: ");
    Serial.println(newVersion);
    http.end();

    // ── PASO 2: Comparar con versión local ───────────────────────────────
    if (newVersion == VERSION_LOCAL) {
        Serial.println("✓ Firmware actualizado, no hay nada que hacer");
        return;
    }

    // ── PASO 3: Descargar firmware .bin desde GitHub releases ────────────
    Serial.println("Nueva versión detectada, descargando firmware...");
    delay(500);

    http.begin(client, FIRMWARE_URL);
    int httpCode = http.GET();

    Serial.print("[DEBUG] HTTP code descarga: ");
    Serial.println(httpCode);

    if (httpCode != 200) {
        Serial.print("✗ Error al descargar firmware: ");
        Serial.println(httpCode);
        http.end();
        return;
    }

    // ── PASO 4: Flashear firmware con Update ─────────────────────────────
    int len = http.getSize();
    Serial.print("Tamaño firmware: ");
    Serial.println(len);

    if (!Update.begin(len)) {
        Serial.println("✗ No se pudo iniciar Update (¿espacio insuficiente?)");
        http.end();
        return;
    }

    // Escribir stream directo a flash sin buffer intermedio
    WiFiClient* stream = http.getStreamPtr();
    size_t written = Update.writeStream(*stream);

    Serial.print("Bytes escritos: ");
    Serial.println(written);

    if (!Update.end()) {
        Serial.print("✗ Update error: ");
        Serial.println(Update.getError());
        http.end();
        return;
    }

    Serial.println("✓ ¡Firmware actualizado exitosamente!");
    http.end();
    delay(1000);
    ESP.restart();  // reinicia para aplicar el nuevo firmware
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    pinMode(2, OUTPUT);

    Serial.println("\n=== OTA desde GitHub ===");

    // Conectar a WiFi
    WiFi.begin("NAVITRACK -2.4G", "N@V1TR4CK");
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.print("\nIP: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("\n✗ No se pudo conectar al WiFi");
    }
}

void loop() {
    // Parpadeo LED para confirmar que el firmware está corriendo
    digitalWrite(2, HIGH);
    Serial.println("LED ON");
    delay(100);

    digitalWrite(2, LOW);
    Serial.println("LED OFF");
    delay(1000);

    // Verificar actualización cada 10 segundos
    if (millis() - lastCheck > 10000) {
        lastCheck = millis();
        checkAndUpdate();
    }
}