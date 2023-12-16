// Librerías a usar.
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

// Pin al que está conectado el sensor DHT22.
#define DHTPIN 2
// Tipo de sensor DHT (en este caso es el DHT22). 
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);

// Dirección I2C de la pantalla OLED.
#define i2c_Address 0x3C 

// Dimensiones de la pantalla OLED.
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
// Pin de reset de la pantalla OLED.
#define OLED_RESET -1    
Adafruit_SH1106G display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Definimos los pines.
int mq135_analog_pin = A0;
int ssr_pin = 4;
int fan_pin = 5;
int relay_gas_pin = 6;

// Pines de los potenciómetros.
int pot1_pin = A1; // Porcentaje CO2 potenciómetro.
int pot2_pin = A2; // Temperatura potenciómetro.

// Convertimos la lectura analógica del sensor MQ135 a porcentaje de CO2.
float analogToPercentageCO2(int analogValue);

void setup() {
  Serial.begin(9600);
  dht.begin();
  pinMode(ssr_pin, OUTPUT);
  pinMode(fan_pin, OUTPUT);
  pinMode(relay_gas_pin, OUTPUT);

  pinMode(pot1_pin, INPUT);
  pinMode(pot2_pin, INPUT);

  // Inicializamos la pantalla OLED.
  if (!display.begin(i2c_Address, true)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;
  }
  display.display();
  delay(0);
  display.clearDisplay();
}

void loop() {
  // Lectura de los sensores.
  float temperatura = dht.readTemperature();
  float humedad = dht.readHumidity();
  int sensorValue = (analogRead(mq135_analog_pin) * 17.6) / 100; // Calibración del sensor MQ135
  float percentageCO2 = analogToPercentageCO2(sensorValue);

  // Lectura de los potenciómetros.
  int pot1_value = analogRead(pot1_pin);
  int pot2_value = analogRead(pot2_pin);

  // Damos rangos de temperaturas y CO2.
  int temperature_limit = map(pot2_value, -1, 1023, 20, 41);
  int co2_limit = map(pot1_value, -2, 1023, 0, 102);

  // Actualizamos la pantalla OLED con las mediciones y configuraciones.
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(0, 0);
  display.print(F("Temp. real: "));
  display.print(temperatura);
  display.print(F(" C"));

  display.setCursor(0, 10);
  display.print(F("% Humedad: "));
  display.print(humedad);
  display.print(F(" %"));

  display.setCursor(0, 20);
  display.print(F("% CO2 real: "));
  display.print(sensorValue);
  display.print(F(" %"));

  display.setCursor(0, 40);
  display.print(F("Temp. esperada: "));
  display.print(temperature_limit);
  display.print(F(" C"));

  display.setCursor(0, 50);
  display.print(F("% CO2 esperado: "));
  display.print(co2_limit);
  display.print(F(" %"));

  display.display();

  // Controlamos el relé de gas en función de la concentración de CO2.
  if (sensorValue < co2_limit) {
    digitalWrite(relay_gas_pin, HIGH);
  } else {
    digitalWrite(relay_gas_pin, LOW);
  }

  static unsigned long previousMillis = 0;
  const unsigned long highInterval = 20000; // Damos 20 segundos.

  unsigned long lowInterval;

  int temperatureDifference = temperature_limit - temperatura;

  if (temperatura <= 30) {                                                // Si se tiene temperatura menor o igual a 30.
    if (temperatureDifference <= 1) {                                     // 1 segundo prende.
      lowInterval = 1000;
    } else if (temperatureDifference > 1 && temperatureDifference <= 6) { // 2 segundos prende.
      lowInterval = 2000;
    } else if (temperatureDifference > 6) {                               // 4 segundos prende.
      lowInterval = 4000;
    } else if (temperatureDifference <= 0) {                              // No prende.
    } else {
 
      lowInterval = 1000;
    }
  } else {                                                                // Si se tiene temperatura nayor a 30.
    if (temperatureDifference <= 1) {                                     // 2.25 segundos prende.
      lowInterval = 2250;
    } else if (temperatureDifference > 1) {                               // 6 segundos prende.
      lowInterval = 6000;
    } else if (temperatureDifference <= 0) {
      lowInterval = 0;
    } else {

      lowInterval = 2000;
    }
  }

  // Si la temperatura es mayor que el límite.
  if (temperatura < temperature_limit) {
    unsigned long currentMillis = millis();

    if (currentMillis - previousMillis >= highInterval && digitalRead(ssr_pin) == LOW) {
      previousMillis = currentMillis;
      digitalWrite(ssr_pin, HIGH);
    }

    if (currentMillis - previousMillis >= lowInterval && digitalRead(ssr_pin) == HIGH) {
      previousMillis = currentMillis;
      digitalWrite(ssr_pin, LOW);
    }

    digitalWrite(fan_pin, HIGH);
  } else {
    digitalWrite(ssr_pin, LOW);
    digitalWrite(fan_pin, LOW); 
  }

  delay(0);
}

// Convertimos la lectura analógica del sensor MQ135 a porcentaje de CO2.
float analogToPercentageCO2(int analogValue) {
  // Calibramos el sensor MQ135 para CO2.
  float Ro = 0.01;                // Valor de resistencia en aire limpio.
  float analogValueCO2 = 10000.0; // Lectura analógica en concentración conocida de CO2.

  // Cálculo del porcentaje de CO2.
  float percentageCO2 = analogValue * Ro;

  return percentageCO2;
}
