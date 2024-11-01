#include <LiquidCrystal_I2C.h> //Biblioteca do display
#include <RTClib.h> // Biblioteca para Relógio em Tempo Real (RTC)
#include <Wire.h>   // Biblioteca para comunicação I2C
#include <EEPROM.h> // Biblioteca para acesso à memória EEPROM
#include "DHT.h"    // Biblioteca para sensor de temperatura e umidade DHT

#define LOG_OPTION 1     // Opção para ativar a leitura do log
#define SERIAL_OPTION 0  // Opção de comunicação serial: 0 para desligado, 1 para ligado
#define UTC_OFFSET -3    // Ajuste de fuso horário para UTC-3

// Configurações do DHT22 (Wokwi)
#define DHTPIN 6
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// Define o tipo de dislay
LiquidCrystal_I2C lcd(0x27, 16, 2); 
RTC_DS1307 RTC; //Relógio

// Configurações da EEPROM
const int maxRecords = 20;
const int recordSize = 8; // Tamanho de cada registro em bytes
int startAddress = 0;
int endAddress = maxRecords * recordSize;
int currentAddress = 0;
int lastLoggedMinute = -1;
const unsigned long logInterval = 10000; // Intervalo de gravação (10 segundos)
unsigned long previousMillis = 0; // Tempo da última gravação

// Triggers de temperatura e umidade
float trigger_t_min = 20.0; // Valor mínimo de temperatura
float trigger_u_min = 30.0; // Valor mínimo de umidade
float trigger_t_max = 30.0; // Valor máximo de temperatura
float trigger_u_max = 70.0; // Valor máximo de umidade

// Associando pinos físicos as suas funções correspondentes
#define pinoLDR A0
#define ledVermelho 2
#define ledAmarelo 3
#define ledVerde 4
#define buzzer 5

// Declarando a variável global do nível de luminosidade 
int intensidadeLDR; 


void setup() {

 //Confiurações para medir luminosidade
 Serial.begin(9600); // Inicializa a TX Serial

 lcd.init();         // Inicializa o LCD
 lcd.backlight();   // Ativa o Backlight do display

 pinMode(pinoLDR, INPUT); //Define o sensor LDR como entrada
 pinMode(ledVermelho, OUTPUT); //Define os LEDs como saída
 pinMode(ledAmarelo, OUTPUT);
 pinMode(ledVerde, OUTPUT);
 pinMode(buzzer, OUTPUT);

 splashScreen();   // Chama a função da animação "MOSKITTO"      


 //Configurações para medir umidade e temperatura
 pinMode(LED_BUILTIN, OUTPUT);
 dht.begin();
 RTC.begin();  // Inicialização do Relógio em Tempo Real
 RTC.adjust(DateTime(F(__DATE__), F(__TIME__)));
 EEPROM.begin();

}

// Função luminosidade e I/O dos LEDs, LDR e buzzer
int luxTick(){
  lcd.clear();
  intensidadeLDR = map(analogRead(pinoLDR), 20, 1010, 0, 100); // função map() usada para criar uma range calibrada para o LDR de 0 a 100

  // Condições da checagem da luminosidade e subsequentes ações
  if (intensidadeLDR > 51) {
    digitalWrite(ledVermelho, HIGH);
    digitalWrite(ledAmarelo, LOW);
    digitalWrite(ledVerde, LOW);
    for (int b=0; b<9; b++){
      delay(200);
      digitalWrite(buzzer, HIGH);
      delay(200);
      digitalWrite(buzzer, LOW);
    }
  }
  if (21 < intensidadeLDR and intensidadeLDR < 50) {
    digitalWrite(ledVermelho, LOW);
    digitalWrite(ledAmarelo, HIGH);
    digitalWrite(ledVerde, LOW);

    for (int a=0; a<4; a++){
      digitalWrite(buzzer, HIGH);
      delay(100);
      digitalWrite(buzzer, LOW);
      delay(500);
    }

  }
  if (intensidadeLDR < 20) {
    digitalWrite(ledVermelho, LOW);
    digitalWrite(ledAmarelo, LOW);
    digitalWrite(ledVerde, HIGH);
  }

}

int umidTemp() { //Função Umidade e Temperatura para o Display
  
  // Ler os valores de temperatura e umidade
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  
  //Converte o valor da temperatura e umidade para caber no display
  int humiInt = (int)humidity;
  char tempStr[6];
  dtostrf(temperature, 4, 1, tempStr);

  //Printar valores das medições no lcd 
  lcd.setCursor(1, 1);
  lcd.print(intensidadeLDR);
  lcd.setCursor(6, 1);
  lcd.print(humiInt);
  lcd.setCursor(11, 1);
  lcd.print(tempStr);
  delay(1000);

  //Caracteres das medições
  byte image03[8] = {B00100, B00100, B01110, B01110, B11111, B11011, B11101, B01110};
  byte image08[8] = {B00000, B00100, B01110, B11101, B11011, B11001, B01110, B00000};
  byte image13[8] = {B00100, B01110, B01010, B01110, B01110, B01110, B01110, B00100};
  byte image32[8] = {B10111, B01000, B01000, B01000, B01000, B01000, B01000, B00111};
  byte image25[8] = {B10001, B00010, B00010, B00100, B00100, B01000, B01000, B10001};
  byte image20[8] = {B10001, B00010, B00010, B00100, B00100, B01000, B01000, B10001};
  lcd.createChar(0, image03);
  lcd.createChar(1, image08);
  lcd.createChar(2, image13);
  lcd.createChar(3, image32);
  lcd.createChar(4, image25);
  lcd.createChar(5, image20);
  lcd.setCursor(2, 0);
  lcd.write(byte(0));
  lcd.setCursor(7, 0);
  lcd.write(byte(1));
  lcd.setCursor(12, 0);
  lcd.write(byte(2));
  lcd.setCursor(15, 1);
  lcd.write(byte(3));
  lcd.setCursor(8, 1);
  lcd.write(byte(4));
  lcd.setCursor(3, 1);
  lcd.write(byte(5));

  
}

int nivelCritico() { //Demonstra o status das medições (ruim ou okay)

  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  int tempInt = (int)temperature;
  int humiInt = (int)humidity;

  if (temperature < trigger_t_min || temperature > trigger_t_max ) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Temp. : ruim");
  }
  else {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Temp. : okay!");
  }

  if (humidity < trigger_u_min || humidity > trigger_u_max){
    lcd.setCursor(0, 1);
    lcd.print("Umidade: ruim");
  }
  else {
    lcd.setCursor(0, 1);
    lcd.print("Umidade: okay!");
  }

}

void relogioLCD() {

  DateTime now = RTC.now();
  DateTime timeIs = DateTime(now);

  // Formatação da exibição do relógio no LCD 16x2
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Data: ");
  lcd.print(timeIs.day() < 10 ? "0" : "");
  lcd.print(timeIs.day());
  lcd.print("/");
  lcd.print(timeIs.month() < 10 ? "0" : "");
  lcd.print(timeIs.month());
  lcd.print("/");
  lcd.print(timeIs.year());

  lcd.setCursor(0, 1);
  lcd.print("Hora: ");
  lcd.print(timeIs.hour() < 10 ? "0" : "");
  lcd.print(timeIs.hour());
  lcd.print(":");
  lcd.print(timeIs.minute() < 10 ? "0" : "");
  lcd.print(timeIs.minute());
  lcd.print(":");
  lcd.print(timeIs.second() < 10 ? "0" : "");
  lcd.print(timeIs.second());
}

// Função de loop principal do sistema para rodar indefinidamente
void loop(){
  
  // Função de indicação do nível da umidade e temperatura
  nivelCritico();
  delay(5000);

  //Função da luminosidade, temperatura e umidade
  luxTick();
  umidTemp();
  delay(5000);

  // Função da exibição do relógio na tela LCD 16x2
  relogioLCD();
  delay(5000); //delay entre as diferentes telas


 //Configurações para armazenar os dados
 DateTime now = RTC.now();

 // Calculando o deslocamento do fuso horário
 int offsetSeconds = UTC_OFFSET * 3600; // Convertendo horas para segundos
 now = now.unixtime() + offsetSeconds; // Adicionando o deslocamento ao tempo atual

 // Convertendo o novo tempo para DateTime
 DateTime adjustedTime = DateTime(now);

  if (LOG_OPTION) get_log();

  unsigned long currentMillis = millis();
    // Verifica se 10 segundos se passaram
    if (currentMillis - previousMillis >= logInterval) {
        previousMillis = currentMillis; // Atualiza o tempo da última gravação

        digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
        delay(1000);                       // wait for a second
        digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
        delay(1000);                       // wait for a second

        // Ler os valores de temperatura e umidade
        float humidity = dht.readHumidity();
        float temperature = dht.readTemperature();

        // Verificar se os valores estão fora dos triggers
        if (temperature < trigger_t_min || temperature > trigger_t_max || humidity < trigger_u_min || humidity > trigger_u_max) {
            // Converter valores para int para armazenamento
            int tempInt = (int)(temperature * 100);
            int humiInt = (int)(humidity * 100);

            // Escrever dados na EEPROM
            EEPROM.put(currentAddress, now.unixtime());
            EEPROM.put(currentAddress + 4, tempInt);
            EEPROM.put(currentAddress + 6, humiInt);

            // Atualiza o endereço para o próximo registro
            getNextAddress();
        }
      get_log();  
    }

    if (SERIAL_OPTION) {
        Serial.print(adjustedTime.day());
        Serial.print("/");
        Serial.print(adjustedTime.month());
        Serial.print("/");
        Serial.print(adjustedTime.year());
        Serial.print(" ");
        Serial.print(adjustedTime.hour() < 10 ? "0" : "");
        Serial.print(adjustedTime.hour());
        Serial.print(":");
        Serial.print(adjustedTime.minute() < 10 ? "0" : "");
        Serial.print(adjustedTime.minute());
        Serial.print(":");
        Serial.print(adjustedTime.second() < 10 ? "0" : "");
        Serial.print(adjustedTime.second());
        Serial.print("\n");
    }

}


void getNextAddress() {
    currentAddress += recordSize;
    if (currentAddress >= endAddress) {
        currentAddress = 0; // Volta para o começo se atingir o limite
    }
}

void get_log() {
    Serial.println("Dados Salvos na EEPROM:");
    Serial.println("Timestamp\t\tTemp. Crítica \tUmid. Crítica");

    for (int address = startAddress; address < endAddress; address += recordSize) {
        long timeStamp;
        int tempInt, humiInt;

        // Ler dados da EEPROM
        EEPROM.get(address, timeStamp);
        EEPROM.get(address + 4, tempInt);
        EEPROM.get(address + 6, humiInt);

        // Converter valores
        float temperature = tempInt / 100.0;
        float humidity = humiInt / 100.0;

        // Verificar se os dados são válidos antes de imprimir
        if (timeStamp != 0xFFFFFFFF) { // 0xFFFFFFFF é o valor padrão de uma EEPROM não inicializada
            //Serial.print(timeStamp);
            DateTime dt = DateTime(timeStamp);
            Serial.print(dt.timestamp(DateTime::TIMESTAMP_FULL));
            Serial.print("\t");
            Serial.print(temperature);
            Serial.print(" C\t\t");
            Serial.print(humidity);
            Serial.println(" %");
        }
    }
}












//Todas essas funções abaixo são os frames do logo "Moskitto"


void splashScreen(){
  image00();//Primeiro frame
  delay(300);
  image01();//Segundo frame
  delay(300);
  image02();//Terceiro frame
  delay(300);
  image03();//Quarto frame
  delay(300);
  image04();//Quinto frame
  delay(300);
  image05();//Sexto frame
  delay(300);
  image06();//Sétimo frame
  delay(300);
  image07();//Oitavo frame
  delay(300);
  image08();//Nono frame
  delay(300);
  image09();//Décimo frame
  delay(300);
  image10();//Décimo primeiro frame
  delay(300);
  image11();//Décimo segundo frame
  delay(300);
  image12();//Décimo terceiro frame
  delay(300);
  image13();//Décimo quarto frame
  delay(300);
  image14();//Décimo quinto frame
  delay(300);
  image15();//Décimo sexto frame
  delay(300);
  image16();//Décimo sétimo frame
  delay(300);
  image17();//Décimo oitavo frame
  delay(300);
  image18();//Décimo nono frame
  delay(300);
  image19();//Vigésimo frame
 	image20(); //Moskitto
}

void image00() { //Primeiro frame
  lcd.clear();
  byte image20[8] = {B11111, B00111, B00010, B00100, B01000, B10000, B00000, B00000};
  byte image21[8] = {B10110, B10100, B11010, B00010, B00001, B00000, B00000, B00000};
  byte image05[8] = {B00000, B00000, B00000, B00001, B00010, B11010, B10100, B10110};
  byte image04[8] = {B00000, B00000, B10000, B01000, B00100, B00010, B00111, B11111};
  byte image03[8] = {B01110, B01001, B00100, B00010, B00010, B00010, B11111, B11111};
  byte image19[8] = {B11111, B11111, B00010, B00010, B00010, B00100, B01001, B01110};
  byte image02[8] = {B00000, B00000, B00000, B00000, B00000, B00000, B00001, B00010};
  byte image18[8] = {B00010, B00001, B00000, B00000, B00000, B00000, B00000, B00000};
  lcd.createChar(0, image20);
  lcd.createChar(1, image21);
  lcd.createChar(2, image05);
  lcd.createChar(3, image04);
  lcd.createChar(4, image03);
  lcd.createChar(5, image19);
  lcd.createChar(6, image02);
  lcd.createChar(7, image18);
  lcd.setCursor(3, 1);
  lcd.write(byte(0));
  lcd.setCursor(4, 1);
  lcd.write(byte(1));
  lcd.setCursor(4, 0);
  lcd.write(byte(2));
  lcd.setCursor(3, 0);
  lcd.write(byte(3));
  lcd.setCursor(2, 0);
  lcd.write(byte(4));
  lcd.setCursor(2, 1);
  lcd.write(byte(5));
  lcd.setCursor(1, 0);
  lcd.write(byte(6));
  lcd.setCursor(1, 1);
  lcd.write(byte(7));
}

void image01() { //Segundo frame
  lcd.clear();
  byte image05[8] = {B00000, B00000, B00000, B10000, B01000, B00101, B01111, B11111};
  byte image06[8] = {B00000, B00000, B00000, B00001, B00010, B10100, B01000, B01100};
  byte image21[8] = {B11111, B01111, B00101, B01000, B10000, B00000, B00000, B00000};
  byte image22[8] = {B01100, B01000, B10100, B00010, B00001, B00000, B00000, B00000};
  byte image20[8] = {B11111, B11111, B00100, B00100, B00100, B01001, B10010, B11100};
  byte image03[8] = {B00000, B00000, B00000, B00000, B00000, B00000, B00001, B00010};
  byte image19[8] = {B00010, B00001, B00000, B00000, B00000, B00000, B00000, B00000};
  byte image04[8] = {B11100, B10010, B01001, B00100, B00100, B00100, B11111, B11111};
  lcd.createChar(0, image05);
  lcd.createChar(1, image06);
  lcd.createChar(2, image21);
  lcd.createChar(3, image22);
  lcd.createChar(4, image20);
  lcd.createChar(5, image03);
  lcd.createChar(6, image19);
  lcd.createChar(7, image04);
  lcd.setCursor(4, 0);
  lcd.write(byte(0));
  lcd.setCursor(5, 0);
  lcd.write(byte(1));
  lcd.setCursor(4, 1);
  lcd.write(byte(2));
  lcd.setCursor(5, 1);
  lcd.write(byte(3));
  lcd.setCursor(3, 1);
  lcd.write(byte(4));
  lcd.setCursor(2, 0);
  lcd.write(byte(5));
  lcd.setCursor(2, 1);
  lcd.write(byte(6));
  lcd.setCursor(3, 0);
  lcd.write(byte(7));
}

void image02() {
  lcd.clear();

  byte image03[8] = {B00000, B00000, B00000, B00000, B00000, B00000, B00000, B00001};
  byte image04[8] = {B00000, B00000, B00000, B00000, B00000, B00100, B11111, B11111};
  byte image19[8] = {B00001, B00000, B00000, B00000, B00000, B00000, B00000, B00000};
  byte image20[8] = {B11111, B11111, B00100, B00000, B00000, B00000, B00000, B00000};
  byte image05[8] = {B00000, B00000, B00000, B00000, B00000, B00101, B01111, B11111};
  byte image21[8] = {B11111, B01111, B00101, B00000, B00000, B00000, B00000, B00000};
  byte image06[8] = {B00000, B00000, B00000, B00001, B00010, B10100, B01000, B01100};
  byte image22[8] = {B01100, B01000, B10100, B00010, B00001, B00000, B00000, B00000};
  lcd.createChar(0, image03);
  lcd.createChar(1, image04);
  lcd.createChar(2, image19);
  lcd.createChar(3, image20);
  lcd.createChar(4, image05);
  lcd.createChar(5, image21);
  lcd.createChar(6, image06);
  lcd.createChar(7, image22);
  lcd.setCursor(2, 0);
  lcd.write(byte(0));
  lcd.setCursor(3, 0);
  lcd.write(byte(1));
  lcd.setCursor(2, 1);
  lcd.write(byte(2));
  lcd.setCursor(3, 1);
  lcd.write(byte(3));
  lcd.setCursor(4, 0);
  lcd.write(byte(4));
  lcd.setCursor(4, 1);
  lcd.write(byte(5));
  lcd.setCursor(5, 0);
  lcd.write(byte(6));
  lcd.setCursor(5, 1);
  lcd.write(byte(7));

}

void image03() { //Quarto Frame
  lcd.clear();

  byte image04[8] = {B00111, B00100, B00010, B00001, B00001, B00001, B01111, B11111};
  byte image20[8] = {B11111, B01111, B00001, B00001, B00001, B00010, B00100, B00111};
  byte image05[8] = {B00000, B10000, B01000, B00100, B10010, B10001, B10011, B11111};
  byte image21[8] = {B11111, B10011, B10001, B10010, B00100, B01000, B10000, B00000};
  byte image22[8] = {B11011, B11010, B01101, B00000, B00000, B00000, B00000, B00000};
  byte image06[8] = {B00000, B00000, B00000, B00000, B00000, B01101, B11010, B11011};
  byte image23[8] = {B00000, B00000, B00000, B00000, B10000, B00000, B00000, B00000};
  byte image07[8] = {B00000, B00000, B00000, B10000, B00000, B00000, B00000, B00000};
  lcd.createChar(0, image04);
  lcd.createChar(1, image20);
  lcd.createChar(2, image05);
  lcd.createChar(3, image21);
  lcd.createChar(4, image22);
  lcd.createChar(5, image06);
  lcd.createChar(6, image23);
  lcd.createChar(7, image07);
  lcd.setCursor(3, 0);
  lcd.write(byte(0));
  lcd.setCursor(3, 1);
  lcd.write(byte(1));
  lcd.setCursor(4, 0);
  lcd.write(byte(2));
  lcd.setCursor(4, 1);
  lcd.write(byte(3));
  lcd.setCursor(5, 1);
  lcd.write(byte(4));
  lcd.setCursor(5, 0);
  lcd.write(byte(5));
  lcd.setCursor(6, 1);
  lcd.write(byte(6));
  lcd.setCursor(6, 0);
  lcd.write(byte(7));

}

void image04() { //Quinto Frame
  lcd.clear();

  byte image20[8] = {B01111, B10111, B01001, B00100, B00011, B00000, B00000, B00000};
  byte image04[8] = {B00000, B00000, B00000, B00011, B00100, B01001, B10111, B01111};
  byte image05[8] = {B00000, B00000, B00000, B11110, B00001, B00001, B11011, B11111};
  byte image21[8] = {B11111, B11011, B00001, B00001, B11110, B00000, B00000, B00000};
  byte image06[8] = {B00000, B00000, B00000, B00000, B00000, B01101, B11010, B11011};
  byte image22[8] = {B11011, B11010, B01101, B00000, B00000, B00000, B00000, B00000};
  byte image07[8] = {B00000, B00000, B00000, B10000, B00000, B00000, B00000, B00000};
  byte image23[8] = {B00000, B00000, B00000, B10000, B00000, B00000, B00000, B00000};
  lcd.createChar(0, image20);
  lcd.createChar(1, image04);
  lcd.createChar(2, image05);
  lcd.createChar(3, image21);
  lcd.createChar(4, image06);
  lcd.createChar(5, image22);
  lcd.createChar(6, image07);
  lcd.createChar(7, image23);
  lcd.setCursor(3, 1);
  lcd.write(byte(0));
  lcd.setCursor(3, 0);
  lcd.write(byte(1));
  lcd.setCursor(4, 0);
  lcd.write(byte(2));
  lcd.setCursor(4, 1);
  lcd.write(byte(3));
  lcd.setCursor(5, 0);
  lcd.write(byte(4));
  lcd.setCursor(5, 1);
  lcd.write(byte(5));
  lcd.setCursor(6, 0);
  lcd.write(byte(6));
  lcd.setCursor(6, 1);
  lcd.write(byte(7));

}

void image05() { //Sexto Frame
  lcd.clear();

  byte image20[8] = {B00111, B00011, B00000, B00000, B00000, B00000, B00001, B00001};
  byte image04[8] = {B00001, B00001, B00000, B00000, B00000, B00000, B00011, B00111};
  byte image05[8] = {B10010, B00001, B10000, B01000, B01010, B01010, B11010, B11011};
  byte image21[8] = {B11011, B11010, B01010, B01010, B01000, B10000, B00001, B10010};
  byte image06[8] = {B00000, B00000, B00000, B10000, B01000, B00101, B01111, B11111};
  byte image22[8] = {B11111, B01111, B00101, B00100, B01000, B10000, B00000, B00000};
  byte image07[8] = {B00000, B00000, B00000, B00000, B00010, B10100, B01000, B01100};
  byte image23[8] = {B01100, B01000, B10100, B00010, B00000, B00000, B00000, B00000};
  lcd.createChar(0, image20);
  lcd.createChar(1, image04);
  lcd.createChar(2, image05);
  lcd.createChar(3, image21);
  lcd.createChar(4, image06);
  lcd.createChar(5, image22);
  lcd.createChar(6, image07);
  lcd.createChar(7, image23);
  lcd.setCursor(3, 1);
  lcd.write(byte(0));
  lcd.setCursor(3, 0);
  lcd.write(byte(1));
  lcd.setCursor(4, 0);
  lcd.write(byte(2));
  lcd.setCursor(4, 1);
  lcd.write(byte(3));
  lcd.setCursor(5, 0);
  lcd.write(byte(4));
  lcd.setCursor(5, 1);
  lcd.write(byte(5));
  lcd.setCursor(6, 0);
  lcd.write(byte(6));
  lcd.setCursor(6, 1);
  lcd.write(byte(7));

}

void image06() { //Sétimo Frame
  lcd.clear();

  byte image20[8] = {B00011, B00001, B00000, B00000, B00000, B00000, B00000, B00000};
  byte image04[8] = {B00000, B00000, B00000, B00000, B00000, B00000, B00001, B00011};
  byte image05[8] = {B01100, B10010, B10001, B01000, B00100, B00100, B11101, B11101};
  byte image21[8] = {B11101, B11101, B00100, B00100, B01000, B10001, B10010, B01100};
  byte image06[8] = {B00000, B00000, B00000, B10000, B01000, B00010, B00111, B11111};
  byte image22[8] = {B11111, B00111, B00010, B01000, B10000, B00000, B00000, B00000};
  byte image23[8] = {B10110, B10100, B11010, B00001, B00000, B00000, B00000, B00000};
  byte image07[8] = {B00000, B00000, B00000, B00000, B00001, B11010, B10100, B10110};


  lcd.createChar(0, image20);
  lcd.createChar(1, image04);
  lcd.createChar(2, image05);
  lcd.createChar(3, image21);
  lcd.createChar(4, image06);
  lcd.createChar(5, image22);
  lcd.createChar(6, image23);
  lcd.createChar(7, image07);


  lcd.setCursor(3, 1);
  lcd.write(byte(0));
  lcd.setCursor(3, 0);
  lcd.write(byte(1));
  lcd.setCursor(4, 0);
  lcd.write(byte(2));
  lcd.setCursor(4, 1);
  lcd.write(byte(3));
  lcd.setCursor(5, 0);
  lcd.write(byte(4));
  lcd.setCursor(5, 1);
  lcd.write(byte(5));
  lcd.setCursor(6, 1);
  lcd.write(byte(6));
  lcd.setCursor(6, 0);
  lcd.write(byte(7));

}

void image07() { //Oitavo Frame
  lcd.clear();

  byte image20[8] = {B00001, B00000, B00000, B00000, B00000, B00000, B00000, B00000};
  byte image04[8] = {B00000, B00000, B00000, B00000, B00000, B00000, B00000, B00001};
  byte image05[8] = {B00000, B00000, B00000, B00111, B01000, B00010, B11110, B11110};
  byte image21[8] = {B11110, B11110, B00010, B01000, B00110, B00000, B00000, B00000};
  byte image22[8] = {B11111, B10011, B00001, B00010, B11100, B00000, B00000, B00000};
  byte image06[8] = {B00000, B00000, B00000, B11100, B00010, B00001, B10011, B11111};
  byte image07[8] = {B00000, B00000, B00000, B00000, B00001, B01100, B11010, B11011};
  byte image23[8] = {B11011, B11010, B01100, B00001, B00000, B00000, B00000, B00000};


  lcd.createChar(0, image20);
  lcd.createChar(1, image04);
  lcd.createChar(2, image05);
  lcd.createChar(3, image21);
  lcd.createChar(4, image22);
  lcd.createChar(5, image06);
  lcd.createChar(6, image07);
  lcd.createChar(7, image23);


  lcd.setCursor(3, 1);
  lcd.write(byte(0));
  lcd.setCursor(3, 0);
  lcd.write(byte(1));
  lcd.setCursor(4, 0);
  lcd.write(byte(2));
  lcd.setCursor(4, 1);
  lcd.write(byte(3));
  lcd.setCursor(5, 1);
  lcd.write(byte(4));
  lcd.setCursor(5, 0);
  lcd.write(byte(5));
  lcd.setCursor(6, 0);
  lcd.write(byte(6));
  lcd.setCursor(6, 1);
  lcd.write(byte(7));

}

void image08() { //Nono frame
  lcd.clear();

  byte image21[8] = {B11111, B01111, B00001, B00001, B00001, B00010, B00100, B00111};
  byte image05[8] = {B00111, B00100, B00010, B00001, B00001, B00001, B01111, B11111};
  byte image22[8] = {B11111, B10011, B10001, B00010, B00100, B01000, B10000, B00000};
  byte image06[8] = {B00000, B10000, B01000, B00100, B00010, B10001, B10011, B11111};
  byte image07[8] = {B00000, B00000, B00000, B00000, B00000, B01101, B11010, B11011};
  byte image23[8] = {B11011, B11010, B01101, B00000, B00000, B00000, B00000, B00000};


  lcd.createChar(0, image21);
  lcd.createChar(1, image05);
  lcd.createChar(2, image22);
  lcd.createChar(3, image06);
  lcd.createChar(4, image07);
  lcd.createChar(5, image23);


  lcd.setCursor(4, 1);
  lcd.write(byte(0));
  lcd.setCursor(4, 0);
  lcd.write(byte(1));
  lcd.setCursor(5, 1);
  lcd.write(byte(2));
  lcd.setCursor(5, 0);
  lcd.write(byte(3));
  lcd.setCursor(6, 0);
  lcd.write(byte(4));
  lcd.setCursor(6, 1);
  lcd.write(byte(5));

}

void image09() {
  lcd.clear();

  byte image21[8] = {B01111, B00111, B00000, B00000, B00010, B00001, B00000, B00000};
  byte image05[8] = {B00000, B00000, B00001, B00010, B00000, B00000, B00111, B01111};
  byte image22[8] = {B10111, B10101, B10101, B10001, B00010, B11100, B00000, B00000};
  byte image06[8] = {B00000, B00000, B11100, B00010, B10001, B10101, B10101, B10111};
  byte image07[8] = {B00000, B00000, B00000, B00000, B00001, B01100, B11010, B11011};
  byte image23[8] = {B11011, B11010, B01100, B00001, B00000, B00000, B00000, B00000};


  lcd.createChar(0, image21);
  lcd.createChar(1, image05);
  lcd.createChar(2, image22);
  lcd.createChar(3, image06);
  lcd.createChar(4, image07);
  lcd.createChar(5, image23);


  lcd.setCursor(4, 1);
  lcd.write(byte(0));
  lcd.setCursor(4, 0);
  lcd.write(byte(1));
  lcd.setCursor(5, 1);
  lcd.write(byte(2));
  lcd.setCursor(5, 0);
  lcd.write(byte(3));
  lcd.setCursor(6, 0);
  lcd.write(byte(4));
  lcd.setCursor(6, 1);
  lcd.write(byte(5));

}

void image10() {
  lcd.clear();

  byte image21[8] = {B00011, B00001, B00000, B00000, B00000, B00000, B00000, B00000};
  byte image05[8] = {B00000, B00000, B00000, B00000, B00000, B00000, B00001, B00011};
  byte image06[8] = {B11110, B10001, B01000, B00100, B00100, B00101, B11101, B11101};
  byte image22[8] = {B11101, B11101, B00101, B00100, B00100, B01000, B10001, B11110};
  byte image07[8] = {B00000, B00000, B10000, B01000, B00100, B00010, B00111, B11111};
  byte image23[8] = {B11111, B00111, B00010, B00100, B01000, B10000, B00000, B00000};
  byte image08[8] = {B00000, B00000, B00000, B00000, B00001, B01010, B10100, B10110};
  byte image24[8] = {B10110, B10100, B01010, B00001, B00000, B00000, B00000, B00000};


  lcd.createChar(0, image21);
  lcd.createChar(1, image05);
  lcd.createChar(2, image06);
  lcd.createChar(3, image22);
  lcd.createChar(4, image07);
  lcd.createChar(5, image23);
  lcd.createChar(6, image08);
  lcd.createChar(7, image24);


  lcd.setCursor(4, 1);
  lcd.write(byte(0));
  lcd.setCursor(4, 0);
  lcd.write(byte(1));
  lcd.setCursor(5, 0);
  lcd.write(byte(2));
  lcd.setCursor(5, 1);
  lcd.write(byte(3));
  lcd.setCursor(6, 0);
  lcd.write(byte(4));
  lcd.setCursor(6, 1);
  lcd.write(byte(5));
  lcd.setCursor(7, 0);
  lcd.write(byte(6));
  lcd.setCursor(7, 1);
  lcd.write(byte(7));

}

void image11() {
  lcd.clear();

  byte image21[8] = {B00001, B00000, B00000, B00000, B00000, B00000, B00000, B00000};
  byte image05[8] = {B00000, B00000, B00000, B00000, B00000, B00000, B00000, B00001};
  byte image06[8] = {B00000, B00000, B00000, B00111, B01000, B00010, B11110, B11110};
  byte image22[8] = {B11110, B11110, B00010, B01000, B00111, B00000, B00000, B00000};
  byte image07[8] = {B00000, B00000, B00000, B11100, B00010, B00001, B10011, B11111};
  byte image23[8] = {B11111, B10011, B00001, B00010, B11100, B00000, B00000, B00000};
  byte image08[8] = {B00000, B00000, B00000, B00000, B00001, B01100, B11010, B11011};
  byte image24[8] = {B11011, B11010, B01100, B00001, B00000, B00000, B00000, B00000};


  lcd.createChar(0, image21);
  lcd.createChar(1, image05);
  lcd.createChar(2, image06);
  lcd.createChar(3, image22);
  lcd.createChar(4, image07);
  lcd.createChar(5, image23);
  lcd.createChar(6, image08);
  lcd.createChar(7, image24);


  lcd.setCursor(4, 1);
  lcd.write(byte(0));
  lcd.setCursor(4, 0);
  lcd.write(byte(1));
  lcd.setCursor(5, 0);
  lcd.write(byte(2));
  lcd.setCursor(5, 1);
  lcd.write(byte(3));
  lcd.setCursor(6, 0);
  lcd.write(byte(4));
  lcd.setCursor(6, 1);
  lcd.write(byte(5));
  lcd.setCursor(7, 0);
  lcd.write(byte(6));
  lcd.setCursor(7, 1);
  lcd.write(byte(7));

}

void image12() {
  lcd.clear();

  byte image06[8] = {B00111, B00100, B00010, B00001, B00001, B00001, B01111, B11111};
  byte image22[8] = {B11111, B01111, B00001, B00001, B00001, B00010, B00100, B00111};
  byte image07[8] = {B00000, B10000, B01000, B00100, B10010, B10001, B10011, B11111};
  byte image23[8] = {B11111, B10011, B10001, B10010, B00100, B01000, B10000, B00000};
  byte image08[8] = {B00000, B00000, B00000, B00000, B00000, B01101, B11010, B11010};
  byte image24[8] = {B11010, B11010, B01101, B00000, B00000, B00000, B00000, B00000};


  lcd.createChar(0, image06);
  lcd.createChar(1, image22);
  lcd.createChar(2, image07);
  lcd.createChar(3, image23);
  lcd.createChar(4, image08);
  lcd.createChar(5, image24);


  lcd.setCursor(5, 0);
  lcd.write(byte(0));
  lcd.setCursor(5, 1);
  lcd.write(byte(1));
  lcd.setCursor(6, 0);
  lcd.write(byte(2));
  lcd.setCursor(6, 1);
  lcd.write(byte(3));
  lcd.setCursor(7, 0);
  lcd.write(byte(4));
  lcd.setCursor(7, 1);
  lcd.write(byte(5));

}

void image13() { //Décimo quarto frame
  lcd.clear();

  byte image06[8] = {B00000, B00000, B00000, B00001, B00010, B00000, B00111, B01111};
  byte image22[8] = {B01111, B00111, B00000, B00010, B00001, B00000, B00000, B00000};
  byte image23[8] = {B10111, B10100, B10001, B00010, B11100, B00000, B00000, B00000};
  byte image07[8] = {B00000, B00000, B00000, B11100, B00010, B10001, B10100, B10111};
  byte image08[8] = {B00000, B00000, B00000, B00000, B00001, B01100, B11010, B11011};
  byte image24[8] = {B11011, B11010, B01100, B00001, B00000, B00000, B00000, B00000};


  lcd.createChar(0, image06);
  lcd.createChar(1, image22);
  lcd.createChar(2, image23);
  lcd.createChar(3, image07);
  lcd.createChar(4, image08);
  lcd.createChar(5, image24);


  lcd.setCursor(5, 0);
  lcd.write(byte(0));
  lcd.setCursor(5, 1);
  lcd.write(byte(1));
  lcd.setCursor(6, 1);
  lcd.write(byte(2));
  lcd.setCursor(6, 0);
  lcd.write(byte(3));
  lcd.setCursor(7, 0);
  lcd.write(byte(4));
  lcd.setCursor(7, 1);
  lcd.write(byte(5));

}

void image14() { //Décimo quinto frame
  lcd.clear();

  byte image22[8] = {B00111, B00011, B00001, B00001, B00001, B00010, B00100, B00111};
  byte image06[8] = {B00111, B00100, B00010, B00001, B00001, B00001, B00011, B00111};
  byte image07[8] = {B10000, B01000, B00100, B00010, B01001, B01000, B11010, B11011};
  byte image23[8] = {B11011, B11010, B01000, B01001, B00010, B00100, B01000, B10000};
  byte image08[8] = {B00000, B00000, B00000, B00000, B00000, B10110, B01101, B11101};
  byte image24[8] = {B11101, B01101, B10110, B00000, B00000, B00000, B00000, B00000};
  byte image09[8] = {B00000, B00000, B00000, B00000, B00000, B01000, B00000, B10000};
  byte image25[8] = {B10000, B00000, B01000, B00000, B00000, B00000, B00000, B00000};


  lcd.createChar(0, image22);
  lcd.createChar(1, image06);
  lcd.createChar(2, image07);
  lcd.createChar(3, image23);
  lcd.createChar(4, image08);
  lcd.createChar(5, image24);
  lcd.createChar(6, image09);
  lcd.createChar(7, image25);


  lcd.setCursor(5, 1);
  lcd.write(byte(0));
  lcd.setCursor(5, 0);
  lcd.write(byte(1));
  lcd.setCursor(6, 0);
  lcd.write(byte(2));
  lcd.setCursor(6, 1);
  lcd.write(byte(3));
  lcd.setCursor(7, 0);
  lcd.write(byte(4));
  lcd.setCursor(7, 1);
  lcd.write(byte(5));
  lcd.setCursor(8, 0);
  lcd.write(byte(6));
  lcd.setCursor(8, 1);
  lcd.write(byte(7));

}

void image15() { //Décimo sexto frame
  lcd.clear();

  byte image22[8] = {B00001, B00000, B00000, B00000, B00000, B00000, B00000, B00000};
  byte image06[8] = {B00000, B00000, B00000, B00000, B00000, B00000, B00000, B00001};
  byte image07[8] = {B00000, B00000, B00000, B00111, B01000, B00010, B11110, B11110};
  byte image23[8] = {B11110, B11110, B00010, B01000, B00111, B00000, B00000, B00000};
  byte image08[8] = {B00000, B00000, B00000, B11100, B00010, B00001, B10011, B11111};
  byte image24[8] = {B11111, B10011, B00001, B00010, B11100, B00000, B00000, B00000};
  byte image25[8] = {B11011, B11010, B01100, B00001, B00000, B00000, B00000, B00000};
  byte image09[8] = {B00000, B00000, B00000, B00000, B00001, B01100, B11010, B11011};


  lcd.createChar(0, image22);
  lcd.createChar(1, image06);
  lcd.createChar(2, image07);
  lcd.createChar(3, image23);
  lcd.createChar(4, image08);
  lcd.createChar(5, image24);
  lcd.createChar(6, image25);
  lcd.createChar(7, image09);


  lcd.setCursor(5, 1);
  lcd.write(byte(0));
  lcd.setCursor(5, 0);
  lcd.write(byte(1));
  lcd.setCursor(6, 0);
  lcd.write(byte(2));
  lcd.setCursor(6, 1);
  lcd.write(byte(3));
  lcd.setCursor(7, 0);
  lcd.write(byte(4));
  lcd.setCursor(7, 1);
  lcd.write(byte(5));
  lcd.setCursor(8, 1);
  lcd.write(byte(6));
  lcd.setCursor(8, 0);
  lcd.write(byte(7));

}

void image16() { //Décimo sètimo frame
  lcd.clear();

  byte image23[8] = {B11111, B01111, B00001, B00001, B00001, B00010, B00100, B00111};
  byte image07[8] = {B00111, B00100, B00010, B00001, B00001, B00001, B01111, B11111};
  byte image24[8] = {B11111, B10011, B10001, B00010, B00100, B01000, B10000, B00000};
  byte image08[8] = {B00000, B10000, B01000, B00100, B00010, B10001, B10011, B11111};
  byte image09[8] = {B00000, B00000, B00000, B00000, B00001, B01100, B11010, B11011};
  byte image25[8] = {B11011, B11010, B01100, B00001, B00000, B00000, B00000, B00000};


  lcd.createChar(0, image23);
  lcd.createChar(1, image07);
  lcd.createChar(2, image24);
  lcd.createChar(3, image08);
  lcd.createChar(4, image09);
  lcd.createChar(5, image25);


  lcd.setCursor(6, 1);
  lcd.write(byte(0));
  lcd.setCursor(6, 0);
  lcd.write(byte(1));
  lcd.setCursor(7, 1);
  lcd.write(byte(2));
  lcd.setCursor(7, 0);
  lcd.write(byte(3));
  lcd.setCursor(8, 0);
  lcd.write(byte(4));
  lcd.setCursor(8, 1);
  lcd.write(byte(5));

}

void image17() { //Décimo oitavo frame
  lcd.clear();

  byte image23[8] = {B01111, B00111, B00000, B00000, B00000, B00001, B00010, B00011};
  byte image07[8] = {B00011, B00010, B00001, B00000, B00000, B00000, B00111, B01111};
  byte image08[8] = {B10000, B01000, B00100, B10010, B10001, B10010, B10000, B10000};
  byte image24[8] = {B10000, B10010, B10000, B10001, B10010, B00100, B01000, B10000};
  byte image25[8] = {B00000, B00001, B00001, B00100, B00010, B00000, B00000, B00000};
  byte image09[8] = {B00000, B00000, B00000, B00100, B00000, B00100, B01001, B00000};


  lcd.createChar(0, image23);
  lcd.createChar(1, image07);
  lcd.createChar(2, image08);
  lcd.createChar(3, image24);
  lcd.createChar(4, image25);
  lcd.createChar(5, image09);


  lcd.setCursor(6, 1);
  lcd.write(byte(0));
  lcd.setCursor(6, 0);
  lcd.write(byte(1));
  lcd.setCursor(7, 0);
  lcd.write(byte(2));
  lcd.setCursor(7, 1);
  lcd.write(byte(3));
  lcd.setCursor(8, 1);
  lcd.write(byte(4));
  lcd.setCursor(8, 0);
  lcd.write(byte(5));

}

void image18() {//Décimo nono frame
  lcd.clear();

  byte image07[8] = {B00001, B00001, B00000, B00000, B00000, B00000, B00011, B00111};
  byte image23[8] = {B00111, B00011, B00000, B00000, B00000, B00000, B00001, B00001};
  byte image08[8] = {B11000, B00110, B10000, B01000, B01000, B01000, B11010, B11010};
  byte image24[8] = {B11000, B11001, B01000, B01000, B01000, B10000, B00100, B11010};
  byte image09[8] = {B00000, B00000, B00000, B01000, B00000, B00000, B00000, B00000};


  lcd.createChar(0, image07);
  lcd.createChar(1, image23);
  lcd.createChar(2, image08);
  lcd.createChar(3, image24);
  lcd.createChar(4, image09);


  lcd.setCursor(6, 0);
  lcd.write(byte(0));
  lcd.setCursor(6, 1);
  lcd.write(byte(1));
  lcd.setCursor(7, 0);
  lcd.write(byte(2));
  lcd.setCursor(7, 1);
  lcd.write(byte(3));
  lcd.setCursor(8, 0);
  lcd.write(byte(4));

}

void image19() {
  lcd.clear();

  byte image07[8] = {B00000, B00000, B00000, B00100, B00000, B00000, B00001, B00011};
  byte image23[8] = {B00011, B00001, B00000, B00100, B00000, B00000, B00000, B00000};
  byte image08[8] = {B00000, B00001, B00000, B00000, B01000, B00000, B10100, B11100};
  byte image24[8] = {B01100, B11100, B00000, B01000, B00000, B00000, B00000, B00010};


  lcd.createChar(0, image07);
  lcd.createChar(1, image23);
  lcd.createChar(2, image08);
  lcd.createChar(3, image24);


  lcd.setCursor(6, 0);
  lcd.write(byte(0));
  lcd.setCursor(6, 1);
  lcd.write(byte(1));
  lcd.setCursor(7, 0);
  lcd.write(byte(2));
  lcd.setCursor(7, 1);
  lcd.write(byte(3));
    
  lcd.clear();
}

void image20() {
  // Liga e desliga a mensagem na segunda linha
  lcd.setCursor(0, 0);
  lcd.print("M O S K I T T O");
  delay(500);
  lcd.clear();
  delay(500);
  lcd.setCursor(0, 1);
  lcd.print("M O S K I T T O");
  delay(500);
  lcd.clear();
  delay(500); // Espera 500 milissegundos
  lcd.print("M O S K I T T O");
  delay(1500); // Espera de 1,5 segundos para iniciar o programa
  lcd.clear();
}
