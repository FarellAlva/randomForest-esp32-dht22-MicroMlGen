#include <WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"
#include "RandomForest.h"  // hasil export dari Python (micromlgen)

const char* ssid = "01galaxy";
const char* password = "1234567890";
const char* mqtt_server = "192.168.43.11";
WiFiClient espClient;
PubSubClient client(espClient);

#define DHTPIN 18       // Pin data DHT22 di ESP32
#define DHTTYPE DHT22  
DHT dht(DHTPIN, DHTTYPE);

Eloquent::ML::Port::RandomForest model;  // model hasil export micromlgen
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
  dht.begin();
  setup_wifi();
  client.setServer(mqtt_server, 1883);
}

void loop() {
  if (!client.connected()) reconnect();
  client.loop();

  // Baca sensor DHT22
  float suhu = dht.readTemperature();     // Celsius
  float kelembaban = dht.readHumidity();  // Persen

  // Cek error pembacaan sensor
  if (isnan(suhu) || isnan(kelembaban)) {
    Serial.println("Gagal membaca dari sensor DHT!");
    delay(2000);
    return;
  }

  // Input ke model [Suhu, Kelembaban] -> urut sesuai training Python
  float input[] = {suhu, kelembaban};

  // Prediksi kelas
  int klasifikasi = model.predict(input);
  String label = getLabel(klasifikasi);

  // Tampilkan hasil ke Serial Monitor
  Serial.print("Suhu: "); Serial.print(suhu, 2); Serial.print(" °C");
  Serial.print(" | Kelembaban: "); Serial.print(kelembaban, 2); Serial.print(" %");
  Serial.print(" | Prediksi: "); Serial.println(label);

  // Kirim ke MQTT dalam format JSON
  char msg[150];
  sprintf(msg, "{\"suhu\": %.2f, \"kelembaban\": %.2f, \"predict\": \"%s\"}",
          suhu, kelembaban, label.c_str());
  client.publish("esp32/dht22", msg);

  delay(1000);
}
