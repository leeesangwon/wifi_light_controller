#include <Debug.h>
#include <JSN270.h>
#include <Arduino.h>
#include <SoftwareSerial.h>
#include <Servo.h>

#define SSID      "Y3.141592"		// your wifi network SSID
#define KEY       "2godnjsqkqh!"		// your wifi network password
#define AUTH       "WPA2" 		// your wifi network security (NONE, WEP, WPA, WPA2)

#define USE_DHCP_IP 0

#if !USE_DHCP_IP
#define MY_IP          "192.168.1.111"
#define SUBNET         "255.255.255.0"
#define GATEWAY        "192.168.1.1"
#endif

#define SERVER_PORT    80
#define PROTOCOL       "TCP"

const int servo_light_off = 9;
const int servo_light_on = 10;
const int min_value = 544;
const int max_value = 2400;

int led_status = 0;
int servo_delay = 1000;

Servo servo;

SoftwareSerial mySerial(3, 2); // RX, TX
 
JSN270 JSN270(&mySerial);

void setup() {
	char c;

	mySerial.begin(9600);
	Serial.begin(9600);

	Serial.println("--------- JSN270 Simple HTTP server Test --------");

	// wait for initilization of JSN270
	delay(5000);
	//JSN270.reset();
	delay(1000);

	//JSN270.prompt();
	JSN270.sendCommand("at+ver\r");
	delay(5);
	while(JSN270.receive((uint8_t *)&c, 1, 1000) > 0) {
		Serial.print((char)c);
	}
	delay(1000);

#if USE_DHCP_IP
	JSN270.dynamicIP();
#else
	JSN270.staticIP(MY_IP, SUBNET, GATEWAY);
#endif    
    
	if (JSN270.join(SSID, KEY, AUTH)) {
		Serial.println("WiFi connect to " SSID);
	}
	else {
		Serial.println("Failed WiFi connect to " SSID);
		Serial.println("Restart System");

		return;
	}    
	delay(1000);

	JSN270.sendCommand("at+wstat\r");
	delay(5);
	while(JSN270.receive((uint8_t *)&c, 1, 1000) > 0) {
		Serial.print((char)c);
	}
	delay(1000);        

	JSN270.sendCommand("at+nstat\r");
	delay(5);
	while(JSN270.receive((uint8_t *)&c, 1, 1000) > 0) {
		Serial.print((char)c);
	}
	delay(1000);
}

void loop() {
	if (!JSN270.server(SERVER_PORT, PROTOCOL)) {
		Serial.println("Failed connect ");
		Serial.println("Restart System");
	} else {
		Serial.println("Waiting for connection...");
	}
      
	String currentLine = "";                // make a String to hold incoming data from the client
	int get_http_request = 0;

	while (1) {
		if (mySerial.overflow()) {
			Serial.println("SoftwareSerial overflow!");
		}
   
		if (JSN270.available() > 0) {
			char c = JSN270.read();
			Serial.print(c);

			if (c == '\n') {                    // if the byte is a newline character
				if (currentLine.length() == 0) {
					if (get_http_request) {
						Serial.println("new client");
						//Serial.println("HTTP RESPONSE");
						// Enter data mode
						JSN270.sendCommand("at+exit\r");
						delay(100);
						
						// HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
						// and a content-type so the client knows what's coming, then a blank line:
						JSN270.println("HTTP/1.1 200 OK");
						JSN270.println("Content-type:text/html");
						JSN270.println();

						// the content of the HTTP response follows the header:
						JSN270.print("Click <a href=\"/H\">here</a> turn the LED on pin 9 on<br>");
						JSN270.print("Click <a href=\"/L\">here</a> turn the LED on pin 9 off<br>");

						// The HTTP response ends with another blank line:
						JSN270.println();

						// wait until all http response data sent:
						delay(1000);

						// Enter command mode:
						JSN270.print("+++");
						delay(100);
						
						// break out of the while loop:
						break;
					}
				}
				// if you got a newline, then clear currentLine:
				else {                
					// Check the client request:
					if (currentLine.startsWith("GET / HTTP")) {
						Serial.println("HTTP REQUEST");
						get_http_request = 1;
					}
					else if (currentLine.startsWith("GET /H")) {
						//Serial.println("GET HIGH");
						get_http_request = 1;
						led_status = 1;

					}
					else if (currentLine.startsWith("GET /L")) {
						//Serial.println("GET LOW");
						get_http_request = 1;
						led_status = -1;
					}
         else if (currentLine.startsWith("GET /D")) {
           //Serial.println("GET LOW");
            get_http_request = 1;
            servo_delay = currentLine.substring(6).toInt();
          }
					currentLine = "";
				}
			}
			else if (c != '\r') {    // if you got anything else but a carriage return character,
				currentLine += c;      // add it to the end of the currentLine
			}
		}
	}

	if (led_status == 1) {
		servo.attach(servo_light_on, min_value, max_value);
    Serial.println("servo on");
    servo.write(60);
    Serial.println("on");
    delay(servo_delay);
    servo.write(105);
    delay(servo_delay);
    servo.detach();
    Serial.println("servo off");
    led_status = 0;
	}
	else if (led_status == -1) {
	  servo.attach(servo_light_off, min_value, max_value);
    Serial.println("servo on");
    servo.write(120);
    Serial.println("on");
    delay(servo_delay);
    servo.write(75);
    delay(servo_delay);
    servo.detach();
    Serial.println("servo off");
    led_status = 0;
	}
	
	// close the connection
	JSN270.sendCommand("at+nclose\r");
	Serial.println("client disonnected");
}

