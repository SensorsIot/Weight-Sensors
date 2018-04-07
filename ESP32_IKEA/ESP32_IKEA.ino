#include "HX711.h"
#include "soc/rtc.h"
#include <WiFi.h>
#include <PubSubClient.h>
#include <credentials.h>

const char* mqttServer = "192.168.0.203";
const int mqttPort = 1883;

HX711 scale;
WiFiClient espClient;
PubSubClient client(espClient);



void setup() {
  Serial.begin(115200);
  rtc_clk_cpu_freq_set(RTC_CPU_FREQ_80M);
  Serial.println("IKEA");
  WiFi.begin(mySSID, myPASSWORD );

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

    if (client.connect("ESP32Client", "admin", "admin" )) {

      Serial.println("connected");

    } else {

      Serial.print("failed with state ");
      Serial.print(client.state());
      Serial.println(" ");
    }
  }

  Serial.println("Initializing the scale");
  // parameter "gain" is ommited; the default value 128 is used by the library

  scale.begin(26, 33);  //  DO,CK


  /*
    // uncomment if you want to calibrate the bowl
    scale. set_scale();
    scale.tare();
    Serial.println("Put known weight on ");
    delay(2500);
    Serial.print(scale.get_units(10));
    Serial.print(" Divide this value to the weight and insert it in the scale.set_scale() statement");
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
  client.publish("ikea/service", "Hello from ESP32");
}

void loop() {

  float averageWeight = (char)scale.get_units(10);
  Serial.print("one reading:\t");
  Serial.print(scale.get_units(), 1);
  Serial.print("\t| average:\t");
  Serial.println(averageWeight);

  char buf[20];
  sprintf(buf, "%04.02f", averageWeight);
  client.publish("ikea/weight", buf);

  client.loop();

  scale.power_down();			        // put the ADC in sleep mode
  delay(1000);
  scale.power_up();
}

