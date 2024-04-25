#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "ARA_ESP.h"

int pin = 2;//Вывод, к которому подключен магнит

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);// Инициализировать последовательный порт
  pinMode(pin, OUTPUT);// Управления выводом, к которому подключен магнит
  esp.begin(Serial);// Запустить движок ESP32
}

int channel = 7;//канал для считывания
void loop() {
// put your main code here, to run repeatedly:
  uint16_t aux = esp.get_channel(channel);// значение с канала приемника, указанного в переменной channel

  if (esp.flag_data)//Проверка на новые данные
  {
    esp.main_f();
    esp.flag_data = false;
  }
  if(aux < 1500){//проверка пришедшее значение 
    digitalWrite(2, LOW);
    // Serial.println("отпуск");
  }
  if(aux >= 1500){
    digitalWrite(2, HIGH);
    // Serial.println("захват");
  }
}
