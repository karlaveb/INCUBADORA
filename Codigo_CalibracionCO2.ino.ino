// Constantes para la fórmula de cálculo de ppm (partes por millón).
const float a = 5.5973021420;
const float b = -0.365425824;
const float Ro = 830; // Valor de referencia en ppm para calibrar el sensor.

// Configuración de hardware
const int RL = 20000;   // Resistencia de carga en ohms.
const int adcPin = A0;  // Pin analógico al que está conectado el sensor.

void setup() {
  Serial.begin(9600); 
}

void loop() {
  float Rs = 1024.0 * float(RL) / analogRead(adcPin) - RL; // Calculamos la resistencia del sensor Rs.
  float R0 = Rs / (a * pow(Ro, b));                        // Calculamos la resistencia de referencia R0.

  float ppm = pow((Rs / R0) / a, 1 / b); // Calculamos las ppm.

  Serial.print("CO2 ppm: ");
  Serial.println(ppm); 

  delay(1000); 
}
