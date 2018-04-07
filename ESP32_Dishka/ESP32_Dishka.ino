#include "HX711.h"
#include "soc/rtc.h"
#include <WiFi.h>
#include <PubSubClient.h>
#include "SSD1306.h"

const char* mqttServer = "192.168.0.203";
const int mqttPort = 1883;

HX711 scale;
WiFiClient espClient;
PubSubClient client(espClient);
SSD1306  display(0x3c, 5, 4);



void setup() {
  Serial.begin(115200);
  rtc_clk_cpu_freq_set(RTC_CPU_FREQ_80M);
  Serial.println("Diska");
  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_24);
  displayString("Welcome", 64, 10);
  delay(100);
  WiFi.begin("*****", "*****" );

  // Wi-Fi
  Serial.println("Connecting to WiFi..");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to the WiFi network");

  // MQTT

  client.setServer(mqttServer, mqttPort);

  while (!client.connected()) {
    Serial.println("Connecting to MQTT...");

    if (client.connect("ESP32Client" )) {

      Serial.println("connected");

    } else {

      Serial.print("failed with state ");
      Serial.print(client.state());
    }
  }

  Serial.println("Initializing the scale");
  // parameter "gain" is ommited; the default value 128 is used by the library
  // HX711.DOUT	- pin #A1
  // HX711.PD_SCK	- pin #A0
  scale.begin(26, 25);


/*
  // uncomment if you want to calibrate the bowl
  scale. set_scale();
  scale.tare();
  Serial.println("Put known weight on ");
  displayString("Calibrate", 64, 10);
  delay(2500);
  Serial.print(scale.get_units(10));
  Serial.print(" Divide this value to the weight and insert it in the scale.set_scale() statement");
  displayFloat(scale.get_units(10), 64, 15);
  while(1==1);

  */

  Serial.println("Before setting up the scale:");
  Serial.print("read: \t\t");
  Serial.println(scale.read());			// print a raw reading from the ADC

  Serial.print("read average: \t\t");
  Serial.println(scale.read_average(20));  	// print the average of 20 readings from the ADC

  Serial.print("get value: \t\t");
  Serial.println(scale.get_value(5));		// print the average of 5 readings from the ADC minus the tare weight (not set yet)

  Serial.print("get units: \t\t");
  Serial.println(scale.get_units(5), 1);	// print the average of 5 readings from the ADC minus tare weight (not set) divided
  // by the SCALE parameter (not set yet)

  scale.set_scale(233.82);    // this value is obtained by calibrating the scale with known weights; see the README for details
  scale.tare();				        // reset the scale to 0

  Serial.println("After setting up the scale:");

  Serial.print("read: \t\t");
  Serial.println(scale.read());                 // print a raw reading from the ADC

  Serial.print("read average: \t\t");
  Serial.println(scale.read_average(20));       // print the average of 20 readings from the ADC

  Serial.print("get value: \t\t");
  Serial.println(scale.get_value(5));		// print the average of 5 readings from the ADC minus the tare weight, set with tare()

  Serial.print("get units: \t\t");
  Serial.println(scale.get_units(5), 1);        // print the average of 5 readings from the ADC minus tare weight, divided
  // by the SCALE parameter set with set_scale

  Serial.println("Readings:");
  client.publish("esp/test", "Hello from ESP32");
}

void loop() {
  Serial.print("one reading:\t");
  Serial.print(scale.get_units(), 1);
  Serial.print("\t| average:\t");
  Serial.println(scale.get_units(10), 1);

  int ww = (int)(scale.get_units(10) + 0.5);

  displayFloat(ww, 64, 15);

  client.loop();

  scale.power_down();			        // put the ADC in sleep mode
  delay(100);
  scale.power_up();
}

void displayFloat(float dispFloat, int x, int y) {
  display.clear();
  display.setColor(WHITE);
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.drawString(64, 15, String(dispFloat, 0));
  display.setFont(ArialMT_Plain_24);
  display.display();
}

void displayString(String dispString, int x, int y) {
  display.clear();
  display.setColor(WHITE);
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.drawString(x, y, dispString);
  display.setFont(ArialMT_Plain_24);
  display.display();
}

