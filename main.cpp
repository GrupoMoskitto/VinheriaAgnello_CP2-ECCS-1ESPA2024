#include "DHT.h"
#define DHT11_PIN 2
 
DHT dht11(DHT11_PIN, DHT11);
 
void setup() {
  Serial.begin(9600);
  dht11.begin(); // Inicializar sensor DHT11
}
 
void loop() {
  delay(2000);
 
  // Umidade
  float humi  = dht11.readHumidity();
  // read temperature as Celsius
  float tempC = dht11.readTemperature();
 
 // Se o sensor não estiver conectado
  if (isnan(humi) || isnan(tempC)) {
    Serial.println("Falha ao ler sensor DHT11!");
  } else {
    Serial.print("Umidade: ");
    Serial.print(humi);
    Serial.print("%");
 
    Serial.print("  |  ");
 
    Serial.print("Temperatura: ");
    Serial.print(tempC);
    Serial.print("°C");
    Serial.println(" ");
  }
}