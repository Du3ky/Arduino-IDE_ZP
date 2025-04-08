#include <Wire.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"

#define FAN_PIN 9

#define TEMP_THRESHOLD 27.0     // Stupňov Celzia
#define HUMIDITY_THRESHOLD 70.0 // % Vlhkosti
#define GAS_THRESHOLD 70000     // Odpor plynov v Ohmoch

Adafruit_BME680 bme;

void setup() {
  Serial.begin(115200);
  while (!Serial);

  pinMode(FAN_PIN, OUTPUT);
  digitalWrite(FAN_PIN, LOW);

  Serial.println("Inicializujem BME680...");
  if (!bme.begin(0x77)) {
    Serial.println("❌ Senzor BME680 sa nenašiel!");
    while (1);
  }

  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(320, 150);
}

void loop() {
  if (!bme.performReading()) {
    Serial.println("❌ Čítanie zlyhalo");
    delay(2000);
    return;
  }

  float temp = bme.temperature;
  float hum = bme.humidity;
  float gas = bme.gas_resistance;

  Serial.println("\n--- Meranie BME680 ---");
  Serial.print("Teplota (°C): "); Serial.println(temp, 2);
  Serial.print("Vlhkosť (%RH): "); Serial.println(hum, 2);
  Serial.print("Gas resistance (Ω): "); Serial.println(gas);

  // Riadiaca logika
  bool ventilatoryZAP = false;

  if (temp > TEMP_THRESHOLD) {
    Serial.println("📈 Vysoká teplota → spúšťam ventilátory");
    ventilatoryZAP = true;
  }

  if (hum > HUMIDITY_THRESHOLD) {
    Serial.println("💧 Vysoká vlhkosť → spúšťam ventilátory");
    ventilatoryZAP = true;
  }

  if (gas < GAS_THRESHOLD) {
    Serial.println("☠️ Zlá kvalita vzduchu → spúšťam ventilátory");
    ventilatoryZAP = true;
  }

  digitalWrite(FAN_PIN, ventilatoryZAP ? HIGH : LOW);

  if (!ventilatoryZAP) {
    Serial.println("✅ Vzduch v poriadku – ventilátory vypnuté");
  }

  delay(3000); // každé 3 sekundy nové meranie
}
