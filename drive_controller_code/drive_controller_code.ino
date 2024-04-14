/*
  WiFi UDP Send and Receive String

  This sketch waits for a UDP packet on localPort using the WiFi module.
  When a packet is received an Acknowledge packet is sent to the client on port remotePort

  created 30 December 2012
  by dlf (Metodo2 srl)

  Find the full UNO R4 WiFi Network documentation here:
  https://docs.arduino.cc/tutorials/uno-r4-wifi/wifi-examples#wi-fi-udp-send-receive-string
 */


#include <WiFiS3.h>
#include <Servo.h>

int status = WL_IDLE_STATUS;
//#include "arduino_secrets.h" 
///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = "PLC";        // your network SSID (name)
char pass[] = "abcdef123!";    // your network password (use for WPA, or use as key for WEP)
int keyIndex = 0;            // your network key index number (needed only for WEP)

// char ssid[] = "65DW3";        // your network SSID (name)
// char pass[] = "abcDEF123!";    // your network password (use for WPA, or use as key for WEP)
// int keyIndex = 0;  

WiFiUDP Udp;

IPAddress ip(192,168,0,57);
//IPAddress ip(192,168,168,207);

//drive motors
#define MT1 5
#define MT2 6

//steering servo
#define SteeringPin 9

Servo servo;

unsigned int localPort = 2390;      // local port to listen on

char packetBuffer[256]; //buffer to hold incoming packet
char  ReplyBuffer[] = "acknowledged\n";       // a string to send back

//keeps track of the states for the outputs
unsigned int steer = 90;
unsigned int mt1 = 0;
unsigned int mt2 = 0;

void setupWiFi(){
// check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }

  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("Please upgrade the firmware");
  }

  //set a static ip adress
  WiFi.config(ip);

  // attempt to connect to WiFi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(10000);
  }
  Serial.println("Connected to WiFi");
  printWifiStatus();

  Serial.println("\nStarting connection to server...");
  // if you get a connection, report back via serial:
  Udp.begin(localPort);
}

void setupHardware(){
  //pins
  pinMode(MT1, OUTPUT);
  pinMode(MT2, OUTPUT);

  //add the servo
  servo.attach(SteeringPin);
}

void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(115200);
  // while (!Serial) {
  //   ; // wait for serial port to connect. Needed for native USB port only
  // }
  setupWiFi();
  setupHardware();
}

void loop() {
  handleUDPPackets();
  delay(1);
  updateHardware();
}

void updateHardware(){
  analogWrite(MT1, mt1);  // turn the LED on (HIGH is the voltage level)
  analogWrite(MT2, mt2);
  servo.write(steer);
}

void handleUDPPackets(){
  // if there's data available, read a packet
  int packetSize = Udp.parsePacket();
  if (packetSize) {

    // read the packet into packetBuffer
    int len = Udp.read(packetBuffer, 255);
    if (len > 0) {
      packetBuffer[len] = 0;
    }
    //get the message prefix
    char prefix[] = {packetBuffer[0], packetBuffer[1], packetBuffer[2], packetBuffer[3]};
    
    //put the info into the correct buffer based on its prefix
    if (strcmp(prefix, "STR=") == 0){
      steer = bufToInt(packetBuffer);
    }
    else if(strcmp(prefix, "MT1=") == 0){
      mt1 = bufToInt(packetBuffer);
    }
    else if(strcmp(prefix, "MT2=") == 0){
      mt2 = bufToInt(packetBuffer);
    }
    else{
      Serial.print("bad packet: ");
      Serial.println(packetBuffer);
    }

    //print the statistics
    Serial.print("str = ");
    Serial.println(steer);
    Serial.print("mt1 = ");
    Serial.println(mt1);
    Serial.print("mt2 = ");
    Serial.println(mt2);
    Serial.println("");
  }
}

int bufToInt(char buf[]){
  //put char 4,5,6 into a new buffer
  char text[] = {buf[4], buf[5], buf[6]};
  //convert the buffer into a string
  String Text = String(text);
  //convert the string to int and return
  return Text.toInt();
}


void printWifiStatus() {
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
