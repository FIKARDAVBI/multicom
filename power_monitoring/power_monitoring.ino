/*
 INA226 Bi-directional Current/Power Monitor. Alert with latch Example.
 Read more: http://www.jarzebski.pl/arduino/czujniki-i-sensory/cyfrowy-czujnik-pradu-mocy-ina226.html
 GIT: https://github.com/jarzebski/Arduino-INA226
 Web: http://www.jarzebski.pl
 (c) 2014 by Korneliusz Jarzebski
 */

#include <Wire.h>
#include <INA226.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

INA226 ina;

unsigned long timer;
const int interval = 100;
float energy;

void setup() {
  Serial.begin(115200);

  Serial.println("Initialize INA226");
  Serial.println("-----------------------------------------------");

  // Default INA226 address is 0x40
  ina.begin(0x44);
  lcd.init();
  lcd.backlight();

  // Configure INA226
  ina.configure(INA226_AVERAGES_1, INA226_BUS_CONV_TIME_1100US, INA226_SHUNT_CONV_TIME_1100US, INA226_MODE_SHUNT_BUS_CONT);

  // Calibrate INA226. Rshunt = 0.01 ohm, Max excepted current = 4A
  ina.calibrate(0.01, 4);
  Serial.println("ina225");
  timer = millis();
  // Enable Power Over-Limit Alert
}

void loop() {
  if (millis() - timer > interval) {
    timer = millis();
    float voltage = ina.readBusVoltage();
    float pwr = ina.readBusPower();
    energy += pwr/36000;
    lcd.setCursor(0,0);
    lcd.print(energy,2);
  }
}
