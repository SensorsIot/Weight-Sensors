#include "HX711.h"
#include "soc/rtc.h"
#include <WiFi.h>
#include <IOXhop_FirebaseESP32.h>
#include <credentials.h>

#define FIREBASE_HOST myFIREBASE_HOST  // replace with your credentials
#define FIREBASE_AUTH myFIREBASE_AUTH   // replace with your credentials
#define SETUPPIN 27


HX711 scale;

byte goToSetup = true;

void setup() {
  Serial.begin(115200);
  rtc_clk_cpu_freq_set(RTC_CPU_FREQ_80M);
  for (int i = 0; i < 10; i++) Serial.println("START");
  pinMode(SETUPPIN, INPUT_PULLUP);
  goToSetup = digitalRead(SETUPPIN);
  Serial.println(goToSetup);
  if (goToSetup == 0) Serial.println("Calibrate");
  WiFi.begin(mySSID, myPASSWORD );  //Replace with your credentials
  Serial.println("Connecting to WiFi..");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to the WiFi network");
  Serial.println("Initializing Firebase");
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);

  Serial.println("Initializing the scale");
  // parameter "gain" is ommited; the default value 128 is used by the library
  delay(500);
  scale.begin(26, 33);  //  DO,CK
  if (goToSetup == 0) calibrate();
  scale.set_scale(20099.f);    // this value is obtained by calibrating the scale with known weights; see the README for details
  scale.tare();				        // reset the scale to 0
  Firebase.setFloat("Weight", 0.0);
  // handle error
  if (Firebase.failed()) {
    Serial.print("setting /number failed:");
    Serial.println(Firebase.error());
    return;
  }
}

void loop() {
  float averageWeight = scale.get_units(10);
  if ( digitalRead(SETUPPIN) == 0) scale.tare();
  Serial.print("one reading:\t");
  Serial.print(scale.get_units(1), 1);
  Serial.print("\t| average:\t");
  Serial.println(averageWeight, 1);

  Firebase.pushFloat("Weight", averageWeight);
  // handle error
  if (Firebase.failed()) {
    Serial.print("setting /number failed:");
    Serial.println(Firebase.error());
    return;
  }
  Serial.println("Published");
   scale.power_down();			        // put the ADC in sleep mode
   delay(10000);
   scale.power_up();
}

void calibrate() {
  Serial.println("Calibrate");
  Serial.println("Remove all weight and press button");
  Firebase.pushFloat("Remove all weight and press button", 0.0);
  waitForKeyPressed();
  scale.set_scale();
  scale.tare();
  Serial.println("Put known weight on and press the button again");
  Firebase.pushFloat("Put known weight on and press the button again", 0.0);
  waitForKeyPressed();

  Serial.print("Weight ");

  float hi = Serial.print(scale.get_units(10));

  Serial.println(hi);
  Firebase.pushFloat("constant", hi);
  Firebase.pushFloat("Divide this value to the weight and insert it in the scale.set_scale() statement", 0);
  Firebase.pushFloat(" Reset the microprocessor", 0);
  while (1 == 1)Serial.println("Waiting");

}

void waitForKeyPressed() {
  while (digitalRead(SETUPPIN) == 1) delay(10);
  while (digitalRead(SETUPPIN) == 0) delay(10);
  Serial.println("Key pressed");

}
