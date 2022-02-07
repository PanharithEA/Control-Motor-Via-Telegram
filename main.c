#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>

// Initialize Wifi connection to the router
char ssid[] = "YC Sopheak JR";     // your network SSID (name)
char password[] = "00000000"; // your network key

// Initialize Telegram BOT
#define BOTtoken "5204380064:AAGGQ1-65cj5oISA6VwWFIaLrUdiKEqHhZw"  // your Bot Token (Get from Botfather)
//sung_bot_token:2137501152:AAH8s8mGy_HUaINUpC_dLKR6qkwSTaWgm4s
//----------------------------------------Host & httpsPort
const char* host = "script.google.com";
const int httpsPort = 443;

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);
String GAS_ID = "AKfycbw4gKucMqlVYoseffGwQpH0czBBsz_SoEqcqpg3Rtr_huYChFJNjYzMfxjKyK3W4Fa1MQ"; //--> spreadsheet script ID

int Bot_mtbs = 100; //mean time between scan messages
long Bot_lasttime;   //last time messages' scan has been done
bool Start = false;

#define PWMR 5
#define PWML 4
#define ENCA 14
#define ENCB 12

long prevT = 0;
int32_t posPrev = 0;

float v1Filt = 0;
float v1Prev = 0;

int32_t Pos = 0;
float a = 0.8;
float eintegral = 0;
const int ledPin = 4;
int POTENTIOMETER = A0;
float volt = 0.0;
int ledStatus = 0;
int pwmval = 0;

bool set_pwm = false;

void handleNewMessages(int numNewMessages) {
  Serial.println("handleNewMessages");
  Serial.println(String(numNewMessages));

  for (int i=0; i<numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    String text = bot.messages[i].text;

    String from_name = bot.messages[i].from_name;
    if (from_name == "") from_name = "Guest";

    if (text == "/ON") {
       long currT = micros();
       float deltaT = ((float) (currT-prevT))/1.0e6;
       float velocity1 =((float) (Pos - posPrev))/deltaT;
       posPrev = Pos;
       prevT = currT;

      // Convert count/s to RPM
      float v1 = velocity1/315.0*60.0;

      // Low-pass filter (25 Hz cutoff)
      v1Filt = 0.99*v1Filt + 0.01*v1 ;//+ 0.0728*v1Prev;
      // v1Prev = v1;
      // Set a target
      float vt = 200;   // Set a target

      // Compute the control signal u
      float kp = 5.0;
      float ki = 0.1;
      float e = vt-abs(v1Filt);
      eintegral = eintegral + e*deltaT;

      float u = kp*e + ki*eintegral;
       int dir = 1;
          if (u<0){
              dir = -1;
             }
       int pwr = (int) fabs(u);
          if(pwr > 255){
             pwr = 255;
             }
//      analogWrite(PWMR, 255);
//      digitalWrite(PWML, LOW);
      setMotor(dir,pwr,PWMR,PWML);

      //digitalWrite(ledPin, HIGH);   // turn the LED on (HIGH is the voltage level)
      ledStatus = 1;
      bot.sendSimpleMessage(chat_id, "Motor is Started", "");
    }

    else if (text == "/OFF") {
      ledStatus = 0;
      digitalWrite(PWMR, LOW);
      digitalWrite(PWML, LOW);
      //digitalWrite(ledPin, LOW);    // turn the LED off (LOW is the voltage level)
      bot.sendSimpleMessage(chat_id, "Motor is Stopped", "");
    }
        
    else if (text == "/potentiometer") {
      int val = analogRead(POTENTIOMETER);
      analogWrite(ledPin, val);   // to read the potenetiometer
      volt = (val/1023.0)*3.3;
      String strVolt = String(volt);
      Serial.print(volt);
      Serial.print("\n");
      bot.sendSimpleMessage(chat_id, "Voltage Value: "+ strVolt, "");
      
    }
    else if (text == "/SpeedMotor") {
      set_pwm = true;
      bot.sendSimpleMessage(chat_id, "Send Value 0 - 255", "");
    }
    else if (text == "/status") {
      if(ledStatus){
        bot.sendSimpleMessage(chat_id, "Motor is Started", "");
      } else {
        bot.sendSimpleMessage(chat_id, "Motor is Stopped", "");
      }
    }
    else if (text == "/start") {
      String welcome = "Welcome to Universal Arduino Telegram Bot library, " + from_name + ".%0A%0A";
      welcome += "This is Flash Led Bot example.%0A";
      welcome += "/ON : to switch the Motor Started%0A";
      welcome += "/OFF : to switch the Motor Stopped%0A";
      welcome += "/SpeedMotor : to set the PWM range 0 - 255%0A";
      welcome += "/status : Returns current status of Motor%0A";
      welcome += "/potentiometer : Returns votage range 0-3.3V status of potentiometer%0A";
      bot.sendSimpleMessage(chat_id, welcome, "");
    }
    else if (set_pwm)
    {
      if (text == "0")
        ledStatus = 0;
      else
        ledStatus = 1;
      analogWrite(PWMR, text.toInt());
      bot.sendSimpleMessage(chat_id, "PWM Value Is: " + text, "");
      set_pwm = false;
    }
  }
}

void ICACHE_RAM_ATTR readEncoder1();


void setup() {
  Serial.begin(115200);

  // Set WiFi to station mode and disconnect from an AP if it was Previously
  // connected
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  // attempt to connect to Wifi network:
  Serial.print("Connecting Wifi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

//  Serial.println("");
//  Serial.println("WiFi connected");

  client.setInsecure();
  
  pinMode(PWMR, OUTPUT); // initialize digital ledPin as an output.
  pinMode(PWML, OUTPUT);
  pinMode(ENCA, INPUT);
  pinMode(ENCB, INPUT);
  attachInterrupt(digitalPinToInterrupt(ENCA), readEncoder1, RISING);
  delay(10);
  //digitalWrite(ledPin, HIGH); // initialize pin as off
}

void loop() {

  if (millis() > Bot_lasttime + Bot_mtbs)  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while(numNewMessages) {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }

    Bot_lasttime = millis();
  }

  long currT = micros();
  float deltaT = ((float) (currT-prevT))/1.0e6;
  float velocity1 =((float) (Pos - posPrev))/deltaT;
  posPrev = Pos;
  prevT = currT;




   // Convert count/s to RPM
  float v1 = velocity1/315.0*60.0;



  // Low-pass filter (25 Hz cutoff)
 v1Filt = 0.99*v1Filt + 0.01*v1 ;//+ 0.0728*v1Prev;
 float meas = v1Filt;
// v1Prev = v1;
// Set a target
  float vt = 200;   // Set a target
  float ref = vt;

   // Compute the control signal u
  float kp = 5.0;
  float ki = 0.1;
  float e = vt-abs(v1Filt);
  eintegral = eintegral + e*deltaT;


  float u = kp*e + ki*eintegral;

  
  // Set the motor speed and direction
  int dir = 1;
  if (u<0){
    dir = -1;
  }
  int pwr = (int) fabs(u);
  if(pwr > 255){
    pwr = 255;
  }

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
//      Serial.print(vt);
//      Serial.print(",");
//
//      Serial.println(v1Filt);
}

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

void setMotor(int dir, int pwmVal, int in1, int in2){
  //analogWrite(pwm,pwmVal); // Motor speed
  if(dir == 1){ 
    // Turn one way
    analogWrite(in1,pwmVal);
    digitalWrite(in2,LOW);
  }
  else if(dir == -1){
    // Turn the other way
    digitalWrite(in1,LOW);
    analogWrite(in2,pwmVal);
  }
  else{
    // Or dont turn
    digitalWrite(in1,LOW);
    digitalWrite(in2,LOW);    
  }
}

void readEncoder1(){
  int b = digitalRead(ENCB);
  if(b>0){
    Pos++;
  }
  else{
    Pos--;
  }
}
