#include <WiFi.h>
#include <PubSubClient.h>

// Definições de rede Wi-Fi
const char* ssid = "Gabriel_Mara";   // Substitua pelo nome da sua rede Wi-Fi
const char* password = "G@briel1512"; // Substitua pela senha da sua rede Wi-Fi

// Definições do broker MQTT
const char* mqtt_server = "mqtt.eclipseprojects.io"; // Broker MQTT público
const int mqtt_port = 1883; // Porta para MQTT sem criptografia (use 8883 para MQTT seguro)

// Definições de pinos
#define RELAY_PIN  13  // Pino digital conectado ao relé
#define PIR_PIN 34     // Pino digital conectado ao sensor PIR

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  // Inicializa a comunicação serial
  Serial.begin(115200);

  // Configura os pinos do relé e do sensor PIR
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(PIR_PIN, INPUT);
  digitalWrite(RELAY_PIN, LOW); // Garante que a lâmpada inicie apagada

  // Conecta-se à rede Wi-Fi
  setup_wifi();

  // Configura o cliente MQTT
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  // Aguardar até o ESP32 se conectar ao Wi-Fi
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando-se ao Wi-Fi...");
  }

  Serial.println("Wi-Fi conectado");
  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());
}

void setup_wifi() {
  delay(10);
  Serial.println("Conectando-se ao Wi-Fi...");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("\nWi-Fi conectado");
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Mensagem recebida [");
  Serial.print(topic);
  Serial.print("] ");
  String message = "";

  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  Serial.println(message);

  // Controle do relé com base na mensagem recebida
  if (message == "ligar") {
    digitalWrite(RELAY_PIN, HIGH);  // Liga a lâmpada
    Serial.println("Lâmpada ligada");
  } else if (message == "desligar") {
    digitalWrite(RELAY_PIN, LOW);   // Desliga a lâmpada
    Serial.println("Lâmpada desligada");
  } else {
    Serial.println("Comando desconhecido!");
  }
}

void reconnect() {
  // Loop de reconexão MQTT
  while (!client.connected()) {
    Serial.print("Conectando ao broker MQTT...");
    if (client.connect("ESP32Client")) {
      Serial.println("Conectado ao broker MQTT");
      client.subscribe("empresa/iluminacao"); // Inscreve-se no tópico
    } else {
      Serial.print("Falha, rc=");
      Serial.print(client.state());
      Serial.println("; tentará novamente em 5 segundos");
      delay(5000);
    }
  }
}

void loop() {
  // Verifica o estado do sensor PIR
  int pirState = digitalRead(PIR_PIN);

  if (pirState == HIGH) { // Se houver movimento detectado
    Serial.println("Movimento detectado! Lâmpada acesa.");
    digitalWrite(RELAY_PIN, HIGH); // Liga a lâmpada
  } else {
    Serial.println("Sem movimento. Lâmpada apagada.");
    digitalWrite(RELAY_PIN, LOW); // Desliga a lâmpada
  }

  // Mantém a comunicação MQTT ativa
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}
