#include <ESP8266HTTPClient.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include <ESP8266WiFi.h>
#include <DebugMacros.h>
#include <HTTPSRedirect.h>
#include <NewPing.h>

#define ONE_WIRE_BUS 2 

// NodeMCU to Arduino pinout conversion:
#define D0 16
#define D1 5
#define D2 4
#define D3 0
#define D4 2
#define D5 14
#define D6 12
#define D7 13
#define D8 15
#define D9 3 
#define D10 1

extern "C" {
#include <cont.h>
  extern cont_t g_cont;
}


OneWire oneWire(ONE_WIRE_BUS); 
DallasTemperature sensors(&oneWire);
NewPing sonar(D8, D6, 200); // NewPing setup of pins and maximum distance.

// Network Credentials for Georgia Tech Wireless Network
// Choose GTother for NodeMCU, Raspberry Pi, or other wifi-enabled 
// Microcontroller/Microprocessor Units
// MAKE SURE TO GET THE CORRECT MAC ADDRESS FOR THE NODEMCU

const char* ssid = "GTother";
const char* password = "GeorgeP@1927";
const char* mac = "5C:CF:7F:A4:28:78";

const char* host = "script.google.com";
// Replace with your own script id to make server side changes
const char *GScriptId = "AKfycbzLbPSSnCXWE3XqGUrFFSr1H2TokeX0UfZRHWMLmymDzVb-1Ll9";

const int httpsPort = 443;

// echo | openssl s_client -connect script.google.com:443 |& openssl x509 -fingerprint -noout
const char* fingerprint = "E4:7B:1F:46:3C:3E:E8:98:F1:4B:41:A5:AF:85:19:7D:E6:2A:73:DE";

// Write to Google Spreadsheet
String url = String("/macros/s/") + GScriptId + "/exec?value=";

HTTPSRedirect* client = NULL;


void setup() {
  // put your setup code here, to run once:
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
  Serial.flush();
    
  Serial.println();
  Serial.print("MAC Address: ");
  Serial.println(WiFi.macAddress());
  Serial.print("Connecting to wifi: ");
  Serial.println(ssid);
  // flush() is needed to print the above (connecting...) message reliably, 
  // in case the wireless connection doesn't go through
  Serial.flush();
  //WiFi.begin(ssid, password);
  WiFi.begin(ssid);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  
// Use HTTPSRedirect class to create a new TLS connection
  HTTPSRedirect* client = new HTTPSRedirect(httpsPort);
  client->setPrintResponseBody(true);
  Serial.print("Connecting to ");
  Serial.println(host);

  // Try to connect for a maximum of 5 times
  bool flag = false;
  for (int i=0; i<5; i++){
    int retval = client->connect(host, httpsPort);
    if (retval == 1) {
       flag = true;
       break;
    }
    else
      Serial.println("Connection failed. Retrying...");
  }

  if (!flag){
    Serial.print("Could not connect to server: ");
    Serial.println(host);
    Serial.println("Exiting...");
    return;
  }
  
  if (client->verify(fingerprint, host)) {
    Serial.println("Certificate match.");
  } else {
    Serial.println("Certificate mis-match");
  }
  
  //TEST RUN
  client->GET(url+"test",host);
  delete client;
  client = NULL;
}

void loop() {
  Serial.println("Send data to Google Script.");
  sensors.requestTemperatures();
  float tempC = sensors.getTempCByIndex(0);
  unsigned int uS = sonar.ping_cm();
  Serial.print("Temperature is: ");
  Serial.println(tempC);
  Serial.print("Fluid Level (cm) is: ");
  Serial.println(uS);
  
  static int error_count = 0;
  static int connect_count = 0;
  const unsigned int MAX_CONNECT = 20;
  static bool flag = false;  

  client = new HTTPSRedirect(httpsPort);
  while (!client->connected()) {
    Serial.print(".");
    client->connect(host, httpsPort);
  }
  Serial.println();

  // Check to make sure cloud upload is successful
  if(client->GET(url+String(tempC)+"&depth="+String(uS),host)) {
    Serial.print("Successfully uploaded value = ");
    Serial.println(tempC);
    Serial.println();
    // Light the LED to show that the GET request has been completed
    // For nodeMCU, LED LOW means light ON
    digitalWrite(LED_BUILTIN, LOW);
  }
  else {
    Serial.println("Upload failed.");
  }
  delay(2000);
  //LED off
  digitalWrite(LED_BUILTIN, HIGH);
  


  delete client;
  client = NULL;

  delay(2000);
  
  HTTPClient http;
  http.setAuthorization(ssid, password);

  // Use below if no password for WIFI
  //http.setAuthorization(ssid);
  
  // Prepare HTTP POST Request:
  // Provide the Host URL
  http.begin("http://insecure-groker.initialstate.com/api/events");
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  // Provide the Access Key and Bucket Key UNIQUE to the Initial State Account
  http.addHeader("X-IS-AccessKey","NCbUQzFnRPMVoXDSjUL40Paxs0ICSV0Q");
  http.addHeader("X-IS-BucketKey","DGYA4B79R7XM");
  // Post the most recent temperature reading to the Temperature bucket
  http.sendRequest("POST","Temperature="+String(tempC) + "Depth=" + String(uS));
  http.writeToStream(&Serial);
  http.end();
  
  delay(2000);

}
