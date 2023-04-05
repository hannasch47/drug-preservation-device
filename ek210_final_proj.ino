#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <Wire.h>
#include <WiFiNINA.h>
#include <SPI.h>
#include <WiFi.h>
#define SECRET_SSID "BU Guest (unencrypted)" //wifi we will use
#define DHTPIN 2
#define DHTTYPE DHT22
char ssid[] = SECRET_SSID;

//initializing arduino wifi client
WiFiClient client;
int status = WL_IDLE_STATUS;

//email variables
char   HOST_NAME[] = "maker.ifttt.com";
String PATH_NAME   = "/trigger/drug_alert/with/key/hsCC8Tr1CSt0xhzF752Sq"; //EVENT-NAME and YOUR-KEY

DHT dht(DHTPIN, DHTTYPE);
int BLUELED = 5;    //Humidity LED notification
int REDLED = 4;    // Exposed drugs notification
int GREENLED = 6; //  Power notification
int PAD = 10;    //   Hbridge
int HOT = 9;    //    MotorDirectionPin set to pin9
int COLD = 11; //     MotorDirectionPin set to pin11
int messagesent = 0;
float HUM;
float TEMP;
float initTEMP;

void setup() {
  // initialize WiFi connection
  status = WiFi.begin(ssid);
  delay(5000); //wait to establish connection
  
  //test connection and retry if not connecting properly
  while ( status != WL_CONNECTED) {
    Serial.print("Attempting to connect to network, SSID: ");
    Serial.println(ssid);
    status = WiFi.begin(ssid);
    delay(10000); //wait 10 seconds for connection:
  }
  Serial.begin(9600); // start serial port at 9600 bps
  Serial.println(status);
  dht.begin();
  delay(1000);
  pinMode(PAD, OUTPUT);
  digitalWrite(PAD, LOW);
  pinMode(HOT, OUTPUT);  
  pinMode(COLD, OUTPUT);
  pinMode(BLUELED, OUTPUT);
  pinMode(REDLED, OUTPUT);
  pinMode(GREENLED, OUTPUT);
  digitalWrite(GREENLED, HIGH);
  initTEMP = dht.readTemperature();
  
  // connect to web server on port 80:
  if (client.connect(HOST_NAME, 80)) {
    Serial.println("Connected to server"); // if connected
  }
  else {
    Serial.println("Connection to server failed"); // if not connected
  }
}

void loop() {  
  initTEMP = 22.0;
  if (initTEMP < 20.0 || initTEMP > 25.0) {
    while (initTEMP < 20.0) {    
      digitalWrite(PAD, HIGH);
      analogWrite(COLD, 0); // 0-255
      analogWrite(HOT, 150); // 0-255 
      }
    while (initTEMP > 25.0) {
      digitalWrite(PAD, HIGH);
      analogWrite(COLD, 250); // 0-255
      analogWrite(HOT, 0); // 0-255
      }
    delay(300000); //wait for 5 min
    initTEMP = 22.0;
    }
  
  // now device has reached proper conditions within 5 minutes
  HUM = dht.readHumidity(); 
  TEMP = dht.readTemperature();

  //Print temp and hum vals to serial monitor for testing
  Serial.print("Humidity: ");
  Serial.print(HUM);
  Serial.write(" %  Temperature: ");
  Serial.print(TEMP);
  Serial.write(" Celcius\n");
  delay(1000);
  
  if (TEMP <= 12.0 || TEMP >= 28.0) {
    while (TEMP <= 12.0) {
      digitalWrite(PAD, HIGH);
      analogWrite(COLD, 0); // 0-255
      analogWrite(HOT, 150); // 0-255
      delay(1000); //wait for 1 sec 
      Serial.print("Humidity: ");
      Serial.print(HUM);
      Serial.write(" %  Temperature: ");
      Serial.print(TEMP);
      Serial.write(" Celcius\n");
      delay(1000);
      if (TEMP <= 10.0 || HUM >= 60.0) {
        digitalWrite(REDLED, HIGH);
        if (messagesent == 0) {
           sendNotif(TEMP,HUM);
           messagesent = 1;
           }
        }
      if (HUM >= 55.0) {
        digitalWrite(BLUELED, HIGH); //notify users that silica packet needs to be switched
        }
      HUM = dht.readHumidity(); 
      TEMP = dht.readTemperature();
      }
    while (TEMP >= 28.0) {
      digitalWrite(PAD, HIGH);
      analogWrite(COLD, 250); // 0-255
      analogWrite(HOT, 0); // 0-255
      delay(1000); //wait for 1 sec
      Serial.print("Humidity: ");
      Serial.print(HUM);
      Serial.write(" %  Temperature: ");
      Serial.print(TEMP);
      Serial.write(" Celcius\n");
      delay(1000);
      if (TEMP >= 30.0 || HUM >= 60.0) {
        digitalWrite(REDLED, HIGH);
        if (messagesent == 0) {
           sendNotif(TEMP,HUM);
           messagesent = 1;
           }
        }
      if (HUM >= 55.0) {
        digitalWrite(BLUELED, HIGH);     //notify users that their silica packet needs to be switched
        }
      HUM = dht.readHumidity(); 
      TEMP = dht.readTemperature();
      }
  }
  else {
    digitalWrite(PAD, LOW);
    analogWrite(COLD, 0);
    analogWrite(HOT, 0);     
    Serial.print("Humidity: ");
    Serial.print(HUM);
    Serial.write(" %  Temperature: ");
    Serial.print(TEMP);
    Serial.write(" Celcius\n");
    delay(1000);
    if (HUM >= 55.0) {
      digitalWrite(BLUELED, HIGH);     //notify users that their silica packet needs to be switched
      }
    if (HUM >= 60.0) {
      digitalWrite(REDLED, HIGH);
      if (messagesent == 0) {
           sendNotif(TEMP,HUM);
           messagesent = 1;
           }
      }
    HUM = dht.readHumidity(); 
    TEMP = dht.readTemperature();
    }   
  } 

// function for sending notifications
void sendNotif(float TEMP,float HUM) {
  if(client.connect(HOST_NAME,80)){
    String queryString = "?value1=" + String(TEMP) + "&value2=" + String(HUM);
    client.println("GET " + PATH_NAME + queryString + " HTTP/1.1");
    client.println("Host: " + String(HOST_NAME));
    client.println("Connection: close");
    client.println(); // end HTTP header
    while (client.connected()) {
      if (client.available()) {
        // read incoming byte from the server and print it to serial monitor:
        char c = client.read();
        Serial.print(c);
      }
    }
    Serial.println("Notification Sent");
    // the server's disconnected, stop the client:
    client.stop();
    Serial.println();
    Serial.println("disconnected");
    }
}
