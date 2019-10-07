#include <NTPClient.h>
#include <ArduinoJson.h>
#include <Udp.h>
#include <WiFi.h>
#include <WebSocketsServer.h>
#include <WebSockets.h>
#include "FirebaseESP32.h"

// Constants
const char* ssid = "ssid";
const char* password = "password";

#define CARACTER_SPACE ","
uint8_t qtd_num = 0;
uint8_t nodeID = 0;
#define FIREBASE_HOST "FIREBASE_HOST" //Change to your Firebase RTDB project ID e.g. Your_Project_ID.firebaseio.com
#define FIREBASE_AUTH "FIREBASE_AUTH" //Change to your Firebase RTDB secret password
//Define Firebase Data objects
String output = "";
FirebaseData firebaseData;

String pathDados = "dados";

unsigned long time_execution;
String data_received = "";

boolean can_send = false;

// Globals
WebSocketsServer webSocket = WebSocketsServer(81);

/**router variables**/
// WiFiServer server(80);

HTTPClient http;

// Set your Static IP address
IPAddress local_IP(192, 168, 0, 184);
// Set your Gateway IP address
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress dns(8, 8, 8, 8);

// Configurações do Servidor NTP
const char* servidorNTP = "a.st1.ntp.br"; // Servidor NTP para pesquisar a hora

const int fusoHorario = -10800; // Fuso horário em segundos (-03h = -10800 seg)
const int taxaDeAtualizacao = 1800000; // Taxa de atualização do servidor NTP em milisegundos

WiFiUDP ntpUDP; // Declaração do Protocolo UDP
NTPClient timeClient(ntpUDP, servidorNTP, fusoHorario, 60000);
long timestamp = 0;
/**
   ARDUINO JSON INICIALIZAÇÃO
*/
const size_t capacity = JSON_OBJECT_SIZE(1) + 3 * JSON_OBJECT_SIZE(2) + 2 * JSON_OBJECT_SIZE(4) + 250;
//JSON_OBJECT_SIZE(1) + 3 * JSON_OBJECT_SIZE(2) + 2 * JSON_OBJECT_SIZE(4);
//DynamicJsonDocument doc(capacity);
StaticJsonDocument<1000> doc;

String makeJsonValues(String time, String  retorno[]) {

  String returnValue = "";
  doc.clear();
  JsonObject _time_ = doc.createNestedObject(time);

  JsonObject _time__dht22 = _time_.createNestedObject("dht22");
  _time__dht22["humidity"] = retorno[0];
  _time__dht22["temperature"] = retorno[1];

  JsonObject _time__mq2 = _time_.createNestedObject("mq2");
  _time__mq2["analog"] = retorno[4];
  _time__mq2["lpg"] = retorno[7];
  _time__mq2["co"] = retorno[6];

  JsonObject _time__dht11 = _time_.createNestedObject("dht11");
  _time__dht11["humidity"] = retorno[2];
  _time__dht11["temperature"] = retorno[3];

  JsonObject _time__mq7 = _time_.createNestedObject("mq7");
  _time__mq7["analog"] = retorno[5];
  _time__mq7["ppm"] = retorno[8];

  serializeJson(doc, returnValue);
  Serial.println(returnValue);
  return returnValue;
}

// Called when receiving any WebSocket message
void onWebSocketEvent(uint8_t num,
                      WStype_t type,
                      uint8_t * payload,
                      size_t length) {
  // Figure out the type of WebSocket event
  switch (type) {

    // Client has disconnected
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Disconnected!\n", num);
      break;

    // New client has connected
    case WStype_CONNECTED:
      {
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("[%u] Connection from ", num);
        Serial.println(ip.toString());
        webSocket.sendTXT(num, "recebi alguma coisa");
        // qtd_num = 0;
      }
      break;

    // Echo text message back to client
    case WStype_TEXT:
      {
        Serial.printf("[%u] Text: %s\n", num, payload);

        //Tipo String apontando para referência de memória do payload
        String payloadSeemsString = String((char *)payload);

        char copyPayload[payloadSeemsString.length() + 1];
        payloadSeemsString.toCharArray(copyPayload, payloadSeemsString.length());
        data_received = copyPayload;
        can_send=true;

        Serial.println("recebido" + data_received);
        webSocket.sendTXT(num, "more");
        // qtd_num = 0;
      }
      break;

    // For everything else: do nothing
    case WStype_BIN:
      Serial.println("WStype_BIN EVENT");
      break;
    case WStype_ERROR:
      Serial.println("WStype_ERROR EVENT");
      break;
    case WStype_FRAGMENT_TEXT_START:
      Serial.println("WStype_FRAGMENT_TEXT_START EVENT");
      break;
    case WStype_FRAGMENT_BIN_START:
      Serial.println("WStype_FRAGMENT_BIN_START EVENT");
      break;
    case WStype_FRAGMENT:
      Serial.println("WStype_FRAGMENT EVENT");
      break;
    case WStype_FRAGMENT_FIN:
      Serial.println("WStype_FRAGMENT_FIN EVENT");
      break;
    default:
      break;
  }
}

void setup() {

  // Start Serial port
  Serial.begin(115200);

  // Configures static IP address
  if (!WiFi.config(local_IP, gateway, subnet, dns)) {
    Serial.println("STA Failed to configure");
  }

  // Connect to access point
  Serial.println("Connecting");
  WiFi.begin(ssid, password);
  while ( WiFi.status() != WL_CONNECTED ) {
    delay(500);
    Serial.print(".");
  }

  // Print our IP address
  Serial.println("Connected!");
  Serial.print("My IP address: ");
  Serial.println(WiFi.localIP());

  //  // Connect to Wi-Fi network with SSID and password
  //  Serial.print("Setting AP (Access Point)…");
  //  // Remove the password parameter, if you want the AP (Access Point) to be open
  //  WiFi.softAP(ssid, password);
  //
  //  IPAddress IP = WiFi.softAPIP();
  //  Serial.print("AP IP address: ");
  //  Serial.println(IP);
  //
  //  server.begin();


  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);

  Firebase.setBool(firebaseData, "exhaust_fan", false);
  //delay(5000);//send initial string of led status

  // Start WebSocket server and assign callback
  webSocket.begin();
  webSocket.onEvent(onWebSocketEvent);

  // Iniciar cliente de aquisição do tempo
  timeClient.begin();

}

void loop() {

  timeClient.update();
  Serial.println(timeClient.getEpochTime());

  timestamp = timeClient.getEpochTime();

  //  WiFiClient client = server.available();
  //
  //  if (client.connected()) {
  //
  //  }

  // Look for and handle WebSocket data
  webSocket.loop();
  delay(2000);

  if (data_received != "") {
    if (data_received != "connected" && data_received != "connecte") {
      if (can_send) {
        String  retorno[] = {"", "", "", "", "", "", "", "", ""};
        for (int i = 0; i <= 8; i++) {
          retorno[i] = data_received.substring(0, data_received.indexOf(CARACTER_SPACE));
          data_received = data_received.substring(data_received.indexOf(CARACTER_SPACE) + 1);
          //  Serial.println("VALOR RESANTE "+ data_received);
          // Serial.println("valor "+ retorno[i]);
        }

        if (Firebase.updateNode(firebaseData, pathDados + "/sensors"  , makeJsonValues((String)timestamp, retorno))) {
          Serial.println("sucesso");
          can_send=false;
        } else {
          Serial.println("REASON: " + firebaseData.errorReason());
        }
      }
    }
  }

  //  qtd_num = qtd_num + 1;
  //  if(qtd_num>30){
  //    webSocket.disconnect();
  //    delay(5000);
  //    qtd_num=0;
  //  }
  // Serial.printf(" [%u]   ", qtd_num);

}
