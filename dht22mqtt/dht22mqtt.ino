#include <DHT.h>
#include <WiFi.h>
#include <PubSubClient.h>

const char* ssid = "Kangkung Recycle";
const char* password = "123456798";
const char* mqtt_server = "192.168.187.217";
const int mqtt_port = 1883;
const char* mqtt_user = "test1";
const char* mqtt_password = "test1";
const char* publish_topic = "esp32/test"; // Topik untuk mempublish data
const char* subscribe_topic = "esp32/test"; // Topik untuk berlangganan

#define DHTPIN 21
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP32Client", mqtt_user, mqtt_password)) {
      Serial.println("connected");
      // Berlangganan ke topik yang ditentukan
      client.subscribe(subscribe_topic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  
  // Ubah payload menjadi string
  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.println(message);

  // Proses data yang diterima dari MQTT
  if (String(topic) == subscribe_topic) {
    // Membaca data suhu dan kelembaban dari payload
    float temperature, humidity;
    if (sscanf(message.c_str(), "%f,%f", &temperature, &humidity) == 2) {
      // Lakukan sesuatu dengan data yang diterima, misalnya tampilkan di Serial Monitor
      Serial.print("Temperature: ");
      Serial.print(temperature);
      Serial.print(" °C\t Humidity: ");
      Serial.print(humidity);
      Serial.println(" %");
    } else {
      Serial.println("Failed to parse temperature and humidity from payload!");
    }
  }
}

void setup() {
  Serial.begin(115200);
  dht.begin();
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  delay(2000);

  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println(" *C");

  // Mem-publish data ke topik yang ditentukan
  String payload = String(temperature) + "," + String(humidity);
  client.publish(publish_topic, payload.c_str());
}
