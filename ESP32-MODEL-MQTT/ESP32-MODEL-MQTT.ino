#include <WiFi.h>
#include <PubSubClient.h>
#include "RandomForest.h"  // hasil export dari Python

const char* ssid = "01galaxy";
const char* password = "1234567890";
const char* mqtt_server = "192.168.43.11";

WiFiClient espClient;
PubSubClient client(espClient);

Eloquent::ML::Port::RandomForest model; // <--- ubah jadi ini // inisialisasi model dari micromlgen

int pot1Pin = 32;  // kelembaban
int pot2Pin = 35;  // suhu

void setup_wifi() {
  delay(10);
  Serial.begin(115200);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP32Client")) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

String getLabel(int klasifikasi) {
  switch (klasifikasi) {
    case 0: return "Dingin";
    case 1: return "Sejuk Nyaman";
    case 2: return "Nyaman Optimal";
    case 3: return "Hangat Nyaman";
    case 4: return "Panas";
    default: return "Tidak diketahui";
  }
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
}

void loop() {
  if (!client.connected()) reconnect();
  client.loop();

  // Baca sensor (potensiometer)
  int pot1Raw = analogRead(pot1Pin);
  int pot2Raw = analogRead(pot2Pin);

  float pot1Percent = (pot1Raw / 4095.0) * 100.0; // kelembaban
  float pot2Percent = (pot2Raw / 4095.0) * 100.0;  // suhu (skala 0–40°C)

  // Buat input model [Suhu, Kelembaban] -> urut sesuai training Python
  float input[] = {pot2Percent, pot1Percent};

  // Prediksi kelas
  int klasifikasi = model.predict(input);
  String label = getLabel(klasifikasi);

  // Tampilkan ke Serial
  Serial.print(" | Kelembaban: "); Serial.print(pot1Percent, 2);
  Serial.print(" | Suhu: "); Serial.print(pot2Percent, 2);
 
  Serial.print(" | Prediksi: "); Serial.println(label);

  // Kirim ke MQTT (JSON)
  char msg[150];
  sprintf(msg,
    "{\"pot1\": %.2f, \"pot2\": %.2f, \"predict\": \"%s\"}",
    pot1Percent, pot2Percent, label.c_str()
  );

  client.publish("esp32/potensiometer", msg);
  Serial.println(msg);

  delay(1000);
}
