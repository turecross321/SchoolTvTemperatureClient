#include "WiFiS3.h"
#include <DHT11.h>

#define WIFI_SSID "wifiname"
#define WIFI_PASSWORD "wifipassword"

//#define USE_HTTPS  // Comment out this line to use HTTP in development mode

// Server details
#ifdef USE_HTTPS
  #define SERVER "example.com"
  #define SERVER_PORT 443
  WiFiSSLClient client;  // Use SSL client for HTTPS
#else
  #define SERVER "192.168.30.131"
  #define SERVER_PORT 5000
  WiFiClient client;      // Use regular client for HTTP
#endif

#define PASSWORD "uploadtemperature"  // Change this as needed


// Network credentials
char ssid[] = WIFI_SSID;
char pass[] = WIFI_PASSWORD;

DHT11 dht11(7);

const unsigned long postingInterval = 60L * 1000L; // every 60 seconds
unsigned long lastConnectionTime = postingInterval;

void setup() {
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }

  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("Please upgrade the firmware");
  }

  int status = WL_IDLE_STATUS;
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);
  }

  // you're connected now, so print out the status:
  printWifiStatus();
}

void read_request() {
  uint32_t received_data_num = 0;

  while (client.available()) {
    /* actual data reception */
    char c = client.read();
    /* print data to serial port */
    Serial.print(c);
    /* wrap data to 80 columns*/
    received_data_num++;
    if(received_data_num % 80 == 0) { 
      
    }
    
  }  
}

void loop() {
  // if there's incoming data from the net connection.
  // send it out the serial port.  This is for debugging
  // purposes only:
  read_request();
  
  if (millis() - lastConnectionTime > postingInterval) {
    httpRequest();
  }

}


void httpRequest() {
  // close any connection before sending a new request.
  client.stop();

  int temperature = 0;
  int humidity = 0;

  // Attempt to read the temperature and humidity values from the DHT11 sensor.
  int result = dht11.readTemperatureHumidity(temperature, humidity);

  if (result != 0) {
    Serial.println("Error reading temperature!");
    return;
  }

  // if there's a successful connection:
  if (client.connect(SERVER, SERVER_PORT)) {
    Serial.println("connecting...");

    // send the HTTP POST request:
    client.println("POST /temperature/upload HTTP/1.1"); // Adjust the URL path
    client.print("Host: ");
    client.println(SERVER);
    client.print("Password: ");
    client.println(PASSWORD);
    client.println("Content-Type: application/json"); // Set the content type, assuming JSON data
    client.println("Connection: close");
    client.println("User-Agent: TeknikTvArduino/1.0");

    // Create JSON data with actual temperature
    String jsonData = "{\"celsius\":" + String(temperature) + "}";

    // Calculate the content length
    client.print("Content-Length: ");
    client.println(jsonData.length());
    client.println(); // End of headers
    client.println(jsonData); // JSON data
    
    // note the time that the connection was made:
    lastConnectionTime = millis();
  } else {
    // if you couldn't make a connection:
    Serial.println("connection failed");
  }
}


/* -------------------------------------------------------------------------- */
void printWifiStatus() {
/* -------------------------------------------------------------------------- */  
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}
