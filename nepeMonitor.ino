

#include <time.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

#include "DHT.h"

#define DHTPIN  5
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

const char* ssid = "Debug101";
const char* password = "SuperCepat101";

struct tm timeinfo;
ESP8266WebServer server(80);

int checkInterval     = 0;
int timeSinceLastRead = 0;

int cntRead     = 0;

boolean isPompaOn = false;

float avgTemp   = 0;
float avgHumid  = 0;

float lastTemp  = 0;
float lastHumid = 0;

void setup() {
  Serial.begin(115200);
  //  Serial.setTimeout(2000);

  // Wait for serial to initialize.
  while (!Serial) { }

  Serial.print("connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.hostname("nepe");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  configTime(8 * 3600, 0, "pool.ntp.org", "time.nist.gov");
  time_t now = time(nullptr);
  while (now < 8 * 3600 * 2) {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }
  Serial.println("");
  //struct tm timeinfo;
  gmtime_r(&now, &timeinfo);

  Serial.println("Device Started");
  Serial.println("-------------------------------------");
  Serial.println("Running DHT!");
  Serial.println("-------------------------------------");

  server.begin();
  Serial.println("Web server Started..");
  delay(10000);
  Serial.println("Web server Running..");

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  detect();

}

void detect() {
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();

  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) ) {
    Serial.println("Failed to read from DHT sensor!");
    timeSinceLastRead = 0;
    return;
  }

  // UpdateLast Value.
  lastTemp  = t;
  lastHumid = h;

  // Add To AVG
  avgTemp   += t;
  avgHumid  += h;
  cntRead   += 1;
}

void runPump() {

}

void loop() {
  WiFiClient client = server.available();
  
  // Counter Detik.
  checkInterval += 1;
  
  if ( checkInterval >= 3000 ) {
    detect();
  }

  if (client) {
    Serial.println("New client");
    // bolean to locate when the http request ends
    boolean blank_line = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();

        if (c == '\n' && blank_line) {

          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");
          client.println();
          // your actual web page that displays temperature and humidity
          client.println("<!DOCTYPE HTML>");
          client.println("<html>");
          client.println("<head><title>Pompa Nepe</title></head><body><h1>Near Realtime - Temperature and Humidity</h1><hr><h3>Temperature in Celsius: ");
          client.println(lastTemp);
          client.println("*C<br>Humidity: ");
          client.println(lastHumid);
          client.println("%</h3><hr><h1>Average - Temperature and Humidity</h1><h3>Temperature : ");
          client.println(avgTemp/cntRead);
          client.println("*C<br>Humidity: ");
          client.println(avgHumid/cntRead);
          client.println("</h3><hr><hr>");
          client.println("<h1>Nyalain Pompa Manual</h1><button type=\"button\">Click Me!</button>");
          client.println("</body></html>");
          break;
        }
        if (c == '\n') {
          // when starts reading a new line
          blank_line = true;
        }
        else if (c != '\r') {
          // when finds a character on the current line
          blank_line = false;
        }
      }
    }
    // closing the client connection
    delay(1);
    client.stop();
    Serial.println("Client disconnected.");
  }



}
