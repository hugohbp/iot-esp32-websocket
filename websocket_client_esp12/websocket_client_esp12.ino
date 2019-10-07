#include <StringSplitter.h>
#include <DHTesp.h>
#include <SoftwareSerial.h>

/*
   WebSocketClient.ino

    Created on: 24.05.2015

*/

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WebSocketsClient.h>

String data = "";
ESP8266WiFiMulti WiFiMulti;
WebSocketsClient webSocket;

SoftwareSerial s(D6, D5); // (Rx, Tx)

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {

  switch (type) {
    case WStype_DISCONNECTED:
      Serial.printf("[WSc] Disconnected!\n");
      break;
    case WStype_CONNECTED: {
        Serial.printf("[WSc] Connected to url: %s\n", payload);

        // send message to server when Connected
        webSocket.sendTXT("connected");
      }
      break;
    case WStype_TEXT:
      Serial.printf("[WSc] texto recebido do server: %s\n", payload);
      if (String((char *)payload) == "more") {
        // send message to server
        webSocket.sendTXT(data);
        break;
      }
  }

}

void setup() {

  Serial.begin(9600);
  s.begin(9600);

  WiFiMulti.addAP("name", "password");

  WiFi.disconnect();
  while (WiFiMulti.run() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  // server address, port and URL
  webSocket.begin("192.168.0.184", 81);

  //event handler
  webSocket.onEvent(webSocketEvent);
  Serial.print("conectado ao web socket");

  //use HTTP Basic Authorization this is optional remove if not needed
  //webSocket.setAuthorization("user", "Password");

  // try ever 5000 again if connection has failed
  webSocket.setReconnectInterval(5000);

  //  start heartbeat (optional)
  //  ping server every 15000 ms
  //  expect pong from server within 3000 ms
  //  consider connection disconnected if pong is not received 4 times
  webSocket.enableHeartbeat(15000, 3000, 4);

}

/**
   Função que lê uma string da Serial
   e retorna-a
*/
String leStringSerial() {
  String conteudo = "";
  char caractere;

  // Enquanto receber algo pela serial
  while (s.available() > 0) {
    // Lê byte da serial
    caractere = s.read();
    // Ignora caractere de quebra de linha
    if (caractere != '\n') {
      // Concatena valores
      conteudo.concat(caractere);
    }
    // Aguarda buffer serial ler próximo caractere
    delay(10);
  }
  return conteudo;
}

void loop() {

  // Se receber algo pela serial
  if (s.available() > 0) {
    // Lê toda string recebida
    data = leStringSerial();
  }
  webSocket.loop();



}
