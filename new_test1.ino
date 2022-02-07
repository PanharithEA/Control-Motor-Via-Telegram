//----------------------------------------Include the NodeMCU ESP8266 Library
//----------------------------------------see here: https://www.youtube.com/watch?v=8jMr94B8iN0 to add NodeMCU ESP12E ESP8266 library and board (ESP8266 Core SDK)
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
//----------------------------------------

#define ON_Board_LED 2  //--> Defining an On Board LED, used for indicators when the process of connecting to a wifi router

//----------------------------------------SSID and Password of your WiFi router.
const char* ssid = "YC Sopheak JR"; //--> Your wifi name or SSID.
const char* password = "00000000"; //--> Your wifi password.
//----------------------------------------

//----------------------------------------Host & httpsPort
const char* host = "script.google.com";
const int httpsPort = 443;
//----------------------------------------

WiFiClientSecure client; //--> Create a WiFiClientSecure object.

String GAS_ID = "AKfycbw4gKucMqlVYoseffGwQpH0czBBsz_SoEqcqpg3Rtr_huYChFJNjYzMfxjKyK3W4Fa1MQ"; //--> spreadsheet script ID

//============================================================================== void setup
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(1500);
  
  WiFi.begin(ssid, password); //--> Connect to your WiFi router
  Serial.println("");
    
  pinMode(ON_Board_LED,OUTPUT); //--> On Board LED port Direction output
  digitalWrite(ON_Board_LED, HIGH); //--> Turn off Led On Board

  //----------------------------------------Wait for connection
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    //----------------------------------------Make the On Board Flashing LED on the process of connecting to the wifi router.
    digitalWrite(ON_Board_LED, LOW);
    delay(250);
    digitalWrite(ON_Board_LED, HIGH);
    delay(250);
    //----------------------------------------
  }
  //----------------------------------------
  digitalWrite(ON_Board_LED, HIGH); //--> Turn off the On Board LED when it is connected to the wifi router.
  //----------------------------------------If successfully connected to the wifi router, the IP Address that will be visited is displayed in the serial monitor
  Serial.println("");
  Serial.print("Successfully connected to : ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println();
  //----------------------------------------

  client.setInsecure();
}
//==============================================================================
//============================================================================== void loop
void loop() {
  
  int ref = random(1,100); //t
 
  float meas = random(10,2000); //h
  
  // Check if any reads failed and exit early (to try again).
  if (isnan(meas) || isnan(ref)) {
    Serial.println("Failed to read from DHT sensor !");
    delay(500);
    return;
  }
  String Spref = "Speed reference : " + String(ref) + " rpm";
  String Spmeas = "Speed measurement : " + String(meas) + " rpm";
  Serial.println(Spref);
  Serial.println(Spmeas);
  
  sendData(ref, meas); //--> Calls the sendData Subroutine
}
//==============================================================================
//============================================================================== void sendData
// Subroutine for sending data to Google Sheets
void sendData(float Speedref, int Speedmeas) {
  Serial.println("==========");
  Serial.print("connecting to ");
  Serial.println(host);
  
  //----------------------------------------Connect to Google host
  if (!client.connect(host, httpsPort)) {
    Serial.println("connection failed");
    return;
  }
  //----------------------------------------

  //----------------------------------------Processing data and sending data
  String string_speedref =  String(Speedref);
  String string_speedmeas =  String(Speedmeas, DEC); 
  String url = "/macros/s/" + GAS_ID + "/exec?Vr=" + string_speedref + "&Vm=" + string_speedmeas;
  Serial.print("requesting URL: ");
  Serial.println(url);

  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
         "Host: " + host + "\r\n" +
         "User-Agent: BuildFailureDetectorESP8266\r\n" +
         "Connection: close\r\n\r\n");
         
  Serial.println("request sent");
  //----------------------------------------

  //----------------------------------------Checking whether the data was sent successfully or not
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("headers received");
      break;
    }
  }
  String line = client.readStringUntil('\n');
  if (line.startsWith("{\"state\":\"success\"")) {
    Serial.println("esp8266/Arduino CI successfull!");
  } else {
    Serial.println("esp8266/Arduino CI has failed");
  }
  Serial.print("reply was : ");
  Serial.println(line);
  Serial.println("closing connection");
  Serial.println("==========");
  Serial.println();
  //----------------------------------------
} 
//==============================================================================
