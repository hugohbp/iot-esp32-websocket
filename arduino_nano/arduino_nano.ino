/*Blibliotecas sensores medidores qualidade do ar*/
#include <MQ7.h>
#include <MQ2.h>
/*Biblioteca sensor de temperatura*/
#include <DHT.h>
/*SoftwareSerial para mandar informações via serial para outro Arduino*/
#include <SoftwareSerial.h>

/*CONSTANTES*/
#define DHTPIN22 3
#define DHTTYPE22 DHT22 // DHT 22

#define DHTPIN11 4
#define DHTTYPE11 DHT11 // DHT 11

#define CARACTER_SPACE ","

#define MQ2PIN A0
#define MQ7PIN A1


// Conecte pino 1 do sensor (esquerda) ao +5V
// Conecte pino 2 do sensor ao pino de dados definido em seu Arduino
// Conecte pino 4 do sensor ao GND
// Conecte o resistor de 10K entre pin 2 (dados)
// e ao pino 1 (VCC) do sensor
DHT dht22(DHTPIN22, DHTTYPE22);
DHT dht11(DHTPIN11, DHTTYPE11);


int lpg, co, smoke;

MQ2 mq2(MQ2PIN);
MQ7 mq7(MQ7PIN, 5);


SoftwareSerial s(5, 6);

void setup()
{

  Serial.begin(9600);
  s.begin(9600);

  dht22.begin();
  dht11.begin();
  mq2.begin();



}

void loop()
{
  delay(250);

  // A leitura da temperatura e umidade pode levar 250ms!
  // O atraso do sensor pode chegar a 2 segundos.
  float h = dht22.readHumidity();
  float t = dht22.readTemperature();

  float h1 = dht11.readHumidity();
  float t1 = dht11.readTemperature();

  // testa se retorno é valido, caso contrário algo está errado.
  if (isnan(t) || isnan(h) || isnan(t1) || isnan(h1))
  {
    Serial.println("FAILED TO READ DHT'S");
  }
  else
  {

    String infoSensors = "";
    infoSensors.concat(h);
    infoSensors.concat(CARACTER_SPACE);
    infoSensors.concat(t);
    infoSensors.concat(CARACTER_SPACE);

    infoSensors.concat(h1);
    infoSensors.concat(CARACTER_SPACE);
    infoSensors.concat(t1);
    infoSensors.concat(CARACTER_SPACE);

    infoSensors.concat(analogRead(A0));
    infoSensors.concat(CARACTER_SPACE);
    infoSensors.concat(analogRead(A1));
    infoSensors.concat(CARACTER_SPACE);

    infoSensors.concat( mq2.readCO());
    infoSensors.concat(CARACTER_SPACE);
    infoSensors.concat( mq2.readLPG());
    infoSensors.concat(CARACTER_SPACE);

    infoSensors.concat( mq7.getPPM());

    s.println(infoSensors);
    Serial.println(infoSensors);
  }

}
