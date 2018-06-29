
#include <time.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

#include "DHT.h"

#define DHTPIN  5
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

const char* ssid = "Debug101";
const char* password = "SuperCepat101";

//struct tm timeinfo;
ESP8266WebServer server(80);

int checkInterval     = 0;
int timeSinceLastRead = 0;

int cntRead = 0;
int cntTime = 0;

boolean isPumpOn = false;

float avgTemp   = 0;
float avgHumid  = 0;

float lastTemp  = 0;
float lastHumid = 0;

void setup() {
  Serial.begin(115200);
  //  Serial.setTimeout(2000);

  // Wait for serial to initialize.
  while (!Serial) { }

  Serial.print("Connecting to ");
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

  // UNUSED
  //  configTime(8 * 3600, 0, "pool.ntp.org", "time.nist.gov");
  //  time_t now = time(nullptr);
  //  while (now < 8 * 3600 * 2) {
  //    delay(500);
  //    Serial.print(".");
  //    now = time(nullptr);
  //  }
  //  Serial.println("");
  //  //struct tm timeinfo;
  //  gmtime_r(&now, &timeinfo);

  Serial.println("Device Started");
  Serial.println("-------------------------------------");
  Serial.println("Running DHT!");
  Serial.println("-------------------------------------");

  server.begin();

  Serial.println("-------------------------------------");
  Serial.println("Web server Started..");
  delay(10000);

  Serial.println("-------------------------------------");
  Serial.println("Web server Running..");

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  ///// Server handler
  server.on("/", displayStatus);
  server.on("/on", runPump);

  detect();

}

void displayStatus() {
  String page = "";
  page += "<!DOCTYPE html> <html lang='en'> <head> <meta charset='utf-8'> <meta http-equiv='X-UA-Compatible' content='IE=edge'> <meta name='viewport' content='width=device-width, initial-scale=1'> <title>Nepe Monitor</title>";
  page += "<link rel='stylesheet' href='https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/css/bootstrap.min.css' integrity='sha384-BVYiiSIFeK1dGmJRAkycuHAHRg32OmUcww7on3RYdg4Va+PmSTsz/K68vbdEjh4u' crossorigin='anonymous'>";
  page += "<link rel='stylesheet' href='https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/css/bootstrap-theme.min.css' integrity='sha384-rHyoN1iRsVXV4nD0JutlnGaslCJuC7uwjduW9SVrLvRYooPp2bWYgmgJQIXwl/Sp' crossorigin='anonymous'>";
  page += "</head><body> <div class='container theme-showcase' role='main'>";
  page += "<nav class='navbar navbar-inverse navbar-fixed-top'> <div class='container'> <div class='navbar-header'><a class='navbar-brand' href='#'>Nepe Monitor</a></div></div></nav>";
  page += "<br/>&nbsp;<br/>&nbsp;<div class='row'><div class='col-sm-12'><div class='panel panel-primary'><div class='panel-heading'><h3 class='panel-title'>Near Realtime Sensor</h3></div><div class='list-group-item'>";
  page += "<h4 class='list-group-item-heading'>Temperature</h4>";
  page += "<p class='list-group-item-text'>";
  page += lastTemp;
  page += "&#x2103;</p></div><div class='list-group-item'><h4 class='list-group-item-heading'>Humidity</h4>";
  page += "<p class='list-group-item-text'>";
  page += lastHumid;
  page += " %</p></div></div></div></div><div class='row'><div class='col-sm-12'><div class='panel panel-success'><div class='panel-heading'><h3 class='panel-title'>Average Sensor</h3></div>";
  page += "<div class='list-group-item'><h4 class='list-group-item-heading'>Temperature</h4>";
  page += "<p class='list-group-item-text'>";
  page += avgTemp / cntRead;
  page += "&#x2103;</p></div><div class='list-group-item'><h4 class='list-group-item-heading'>Humidity</h4>";
  page += "<p class='list-group-item-text'>";
  page += avgHumid / cntRead;
  page += "</p></div></div></div></div><div class='progress'>";
  page += "<div class='progress-bar progress-bar-striped' role='progressbar' aria-valuenow='";
  page += cntTime;
  page += "' aria-valuemin='0' aria-valuemax='1800000' style='width: ";
  page += (int) ((cntTime / 18000));
  page += "%'>";
  page += "</div></div>";

  if (!isPumpOn) {
    page += "<button type='button' class='btn btn-lg btn-danger col-sm-12'>Set Pump On For 30 Second</button>";
  }

  page += "</div>";
  page += "<script src='https://ajax.googleapis.com/ajax/libs/jquery/1.12.4/jquery.min.js'></script><script>$(function() {$('.btn-danger').click(function() {var ask = confirm('Nyalain Pompa ?');if(ask) {$('.btn-danger').hide(); $.get( 'on', function( ) {location.reload();});}});});</script>";
  page += "<script src='https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/js/bootstrap.min.js' integrity='sha384-Tc5IQib027qvyjSMfHjOMaLkfuWVxZxUPnCJA7l2mCWNIpG9mGCD8wGNIcPD7Txa' crossorigin='anonymous'></script>";
  page += "</body></html>";
  //  page += cntTime;
  //  page += " - ";
  //  page += (String)(cntTime / 18000);

  Serial.println(cntTime / 18000);

  // lastTemp, lastHumid, avgTemp/cntRead, avgHumid/cntRead, 18000, 50);

  server.send ( 200, "text/html", page );
}

void detect() {
  // Always Reset Counter :)
  checkInterval = 0;

  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();

  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) ) {
    Serial.println("Failed to read from DHT sensor!");
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
  server.send(200, "text/plain", "1");

  if ( isPumpOn ) {
    // No Double Exec :)
    return;
  }

  isPumpOn = true;

  // Nyala Selama 10 Detik.

  Serial.println("Pump Run 1");
  digitalWrite(LED_BUILTIN, LOW);
  delay(10000);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(5000);

  Serial.println("Pump Run 2");
  digitalWrite(LED_BUILTIN, LOW);
  delay(10000);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(5000);


  Serial.println("Pump Run 3");
  digitalWrite(LED_BUILTIN, LOW);
  delay(10000);

  // Turn It Of.
  
  Serial.println("Force Pump OFF");
  digitalWrite(LED_BUILTIN, HIGH);

  isPumpOn = false;
  resetCounter();
}

void resetCounter() {
  checkInterval = 0;
  avgTemp       = 0;
  avgHumid      = 0;
  cntRead       = 0;
  cntTime       = 0;

  digitalWrite(LED_BUILTIN, HIGH);
}

void loop() {

  // Counter "Detik" ?
  checkInterval += 1;

  if ( checkInterval >= 3000 ) {
    detect();
  }

  server.handleClient();

  delay(1);
  cntTime += 1;


  // 1 jam ?
  if (cntTime > 2700000) {
    if ( ((avgTemp / cntRead) >= 30) || (( avgHumid / cntRead ) <= 85) ) {
      runPump();
    }

    resetCounter();
    cntTime = 0;
  }
}
