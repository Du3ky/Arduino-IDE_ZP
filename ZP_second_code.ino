#include <Wire.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"

#define FAN_PIN 9

Adafruit_BME680 bme;

unsigned long startTime = 0;
bool delayPassed = false;

void setup() {
  Serial.begin(115200);
  while (!Serial);

  Serial.println(F("Inicializujem BME680..."));
  if (!bme.begin(0x77)) {
    Serial.println(F("❌ CHYBA! Senzor BME680 sa nenašiel."));
    while (1);
  }
  Serial.println(F("✅ Senzor BME680 bol úspešne inicializovaný!"));

  // Nastavenia senzora
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(320, 150);  // 320°C počas 150 ms – nevyhnutné pre VOC meranie

  pinMode(FAN_PIN, OUTPUT);
  digitalWrite(FAN_PIN, LOW);

  // Prvé meranie + zapamätanie času
  Serial.println(F("⏳ Prvé meranie..."));
  bme.performReading();
  startTime = millis();  // Spusti 10-sekundový odpočet
}

void loop() {
  if (!bme.performReading()) {
    Serial.println(F("❌ Nepodarilo sa načítať údaje zo senzora."));
    delay(3000);
    return;
  }

  // Načítanie hodnôt
  float temperature = bme.temperature;
  float humidity = bme.humidity;
  float gasResistance = bme.gas_resistance;

  // Výpis hodnôt
  Serial.println(F("--- Meranie BME680 ---"));
  Serial.print(F("Teplota (°C): ")); Serial.println(temperature);
  Serial.print(F("Vlhkosť (%RH): ")); Serial.println(humidity);
  Serial.print(F("Odpor plynu (Ω): ")); Serial.println(gasResistance);
  Serial.println();

  // Skontroluj, či uplynulo 10 sekúnd od spustenia
  if (!delayPassed && millis() - startTime >= 10000) {
    delayPassed = true;
    Serial.println(F("✅ Uplynulo 10 sekúnd – ventilátory môžu reagovať na podmienky."));
  }

  // Rozhodovanie o zapnutí ventilátorov + výpis dôvodu
  bool fanShouldRun = false;

  if (delayPassed) {
    //teplota v stupňoch celzia
    if (temperature > 28) {
      Serial.println(F("📈 Vysoká teplota → spúšťam ventilátory"));
      fanShouldRun = true;
    }
    //vlhkosť v %
    else if (humidity > 70) {
      Serial.println(F("💧 Vysoká vlhkosť → spúšťam ventilátory"));
      fanShouldRun = true;
    }
    //odpor plynov v ohmoch
    else if (gasResistance < 70000) {
      Serial.println(F("☠️ Zlá kvalita vzduchu → spúšťam ventilátory"));
      fanShouldRun = true;
    }
  }

  digitalWrite(FAN_PIN, fanShouldRun ? HIGH : LOW);

  if (!fanShouldRun && delayPassed) {
    Serial.println(F("✅ Podmienky sú v norme – ventilátory zostávajú vypnuté."));
  }

  delay(3000);  // Meranie každé 3 sekundy
}
