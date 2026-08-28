//Jenna Fontana
//DuckBuddy 
//CPEG/ELEG 298 spring 26 Semester 
//A "study buddy" - that detects how long a person is studying 
//and times a study session for 30 minutes 

#include <Arduino.h>
#include <SoftwareSerial.h>

#define TRIG_PIN 3 // 5   
#define ECHO_PIN 2 // 6
#define LED_RED 13 //D1
#define LED_YELLOW 12//D2
#define LED_BLUE 8 //D3
#define LED_GREEN 7 //D4
#define MAX_PRESENTS_CM 80.0
#define MIN_PRESENTS_CM 10.0
#define RESET_PIN 9
#define UART_TX_PIN 10
#define UART_RX_PIN 11

String IO_USERNAME = "Jennafont";
String IO_KEY = "YOUR_ADAFRUIT_IO_KEY";
String WIFI_SSID   = "UD Devices";
String WIFI_PASS   = "";
String response;

SoftwareSerial espSerial(UART_RX_PIN, UART_TX_PIN);
uint32_t last_iot_ms = 0;
volatile float distance = 0.0; 
volatile bool new_distance =false;

volatile uint32_t study_seconds = 0; 
volatile uint32_t absent_seconds = 0; 
volatile uint32_t last_tick_ms = 0; 
float timing =0.0;

//blinking 
volatile bool is_blinking = false;
 


void update_LED(void){
  if(study_seconds > 0 && study_seconds <= 599) { //0-10minutes
    digitalWrite(LED_GREEN, HIGH);
    is_blinking = false; 
    //turn on green
  }
  else if (study_seconds > 600  && study_seconds <= 1199 ) { // 10 -20 minutes
    digitalWrite(LED_BLUE, HIGH);
    is_blinking = false; 
    digitalWrite(LED_GREEN, LOW);
  //turn on blue 
  } 
  else if ( study_seconds > 1200 && study_seconds <= 1799) { //20-30 minutes
    digitalWrite(LED_YELLOW, HIGH);
    is_blinking = false;
    digitalWrite(LED_BLUE, LOW);
  //turn on yellow 
  } 
  else if(study_seconds > 1800) { //greater then 30 minutes
    digitalWrite(LED_YELLOW, LOW);
    is_blinking = true; 
  //turn on red blink
  }
}
void clear_LEDs() { //turning all the lEDs off
    digitalWrite(LED_GREEN, LOW); 
    digitalWrite(LED_BLUE, LOW);
    digitalWrite(LED_YELLOW, LOW);
    digitalWrite(LED_RED, LOW);
    is_blinking = false;
    Serial.println("Cleared LEDS");
}
void Send_trigger(){ 
    digitalWrite(TRIG_PIN, LOW);  //pin low
    delayMicroseconds(2); //dealy 2 microseconds
    digitalWrite(TRIG_PIN, HIGH); // turns pin high 
    delayMicroseconds(10); //hold it high for 10 microseconds
    digitalWrite(TRIG_PIN, LOW); //turn pin low again 
    timing = pulseIn(ECHO_PIN, HIGH);
    distance =(timing * 0.034) /2;

    Serial.print("Distance: ");
    Serial.println(distance);

}

void setup() { //telling the system what mode each of the pins are in
    Serial.begin(115200); 
    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);
    pinMode(LED_GREEN, OUTPUT);
    pinMode(LED_BLUE, OUTPUT);
    pinMode(LED_YELLOW, OUTPUT);
    pinMode(LED_RED, OUTPUT);
    pinMode(RESET_PIN, OUTPUT);
    digitalWrite(LED_GREEN, HIGH);
    Serial.println("--- DUCK BUDDY SYSTEM ONLINE ---");
    Serial.println("Initializing Pins...");
    Serial.println("Time Starts Now!!!");

    digitalWrite(RESET_PIN, LOW);
    delay(1000);
    digitalWrite(RESET_PIN, HIGH);
    delay(2000);
    espSerial.begin(9600);

    espData("wifi_ssid=" + WIFI_SSID, 4000, true);
    espData("wifi_pass=" + WIFI_PASS, 4000, false);
    espData("io_key=" + IO_KEY, 4000, false);
    espData("io_user=" + IO_USERNAME, 4000, true);
    
    Serial.println("Connecting to Adafruit IO (This can take 60s)...");
    response = espData("setup_io", 60000, true);

    if (response.indexOf("connected") < 0) {
    Serial.println("Connection Failed. Check power and WiFi registration.");
  }
    espData("setup_pubfeed=1,study_seconds", 10000, true);
    espData("setup_pubfeed=2,absent_seconds", 10000, true);
    espData("setup_pubfeed=3,raw_distance", 10000, true);
  
    Serial.println("Setup Complete");
  }

void Presense(float dist) {  
    uint32_t seconds_passed = (millis() - last_tick_ms) / 1000; //getting back time from when it send stuff to esp
    if (seconds_passed == 0) seconds_passed = 1;  
    if(dist < MAX_PRESENTS_CM && dist > MIN_PRESENTS_CM){ 
        study_seconds += seconds_passed;
        Serial.print("study seconds: ");
        Serial.print(study_seconds);
        Serial.println("s");
        absent_seconds = 0; 
    } else { 
        absent_seconds += seconds_passed;
        Serial.print("absent seconds: ");
        Serial.print(absent_seconds);
        Serial.println("s");
        if(absent_seconds > 5) { //if abesent for more then 10s 
            clear_LEDs();
            Serial.println("USER IS GONE");
            study_seconds = 0; 
        }
    }
}
String espData(String command, const long timeout, boolean debug) {
  char c;
  response = "";
  espSerial.println(command);            //send data to ESP8266 using serial UART
  long int time = millis();              //
  while ((time + timeout) > millis()) {  //wait the timeout period sent with the command
    while (espSerial.available()) {      //look for response from ESP8266
      c = espSerial.read();
      response += c;
    }
    if ((c == '\n') & (command != "setup_io")) break;
  }
  response.trim();
  if (debug) {
    Serial.println("Resp: " + response);
  }
  return response;
}

void loop() {
  // put your main code here, to run repeatedly:
  uint32_t current_ms = millis();
    if(new_distance) {
        new_distance = false; 
    }
    if(millis()- last_tick_ms >= 1000) { //sends out every 1 second
        Send_trigger(); 
        Presense(distance); 
        last_tick_ms = millis();
        if(is_blinking) {
          for(int i = 0; i < 3; i++) {
            digitalWrite(LED_RED, HIGH);
            delay(150); 
            digitalWrite(LED_RED, LOW);
            delay(150); 
          }
        } 
    } 
     update_LED();//update in loop 
     if (current_ms - last_iot_ms >= 8000) {
        last_iot_ms = current_ms;
        Serial.println(">>> Sending Data to Adafruit IO...");
        espData("send_data=1," + String(study_seconds), 2000, true);
        espData("send_data=2," + String(absent_seconds), 2000, true);
        espData("send_data=3," + String(distance), 4000, true);
    }
}
