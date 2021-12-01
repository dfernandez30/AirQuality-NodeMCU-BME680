#define BLYNK_PRINT Serial
#define BLYNK_TEMPLATE_ID   "TU ID AQUÍ"
#include "bsec.h"
#include <Blynk.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

char auth[] = "TU API KEY";

// WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "ssid";
char pass[] = "pass";

// Helper functions declarations
void checkIaqSensorStatus(void);
void errLeds(void);

// Create an object of the class Bsec
Bsec iaqSensor;
 
String output;

void setup(void)
{
  // Debug console
  Serial.begin(115200);
  Wire.begin(4, 2);
  Blynk.begin(auth, ssid, pass);

  iaqSensor.begin(BME680_I2C_ADDR_PRIMARY, Wire);
  output = "\nBSEC library version " + String(iaqSensor.version.major) + "." + String(iaqSensor.version.minor) + "." + String(iaqSensor.version.major_bugfix) + "." + String(iaqSensor.version.minor_bugfix);
  Serial.println(output);
  checkIaqSensorStatus();
  wifi_status_led_uninstall();
 
  bsec_virtual_sensor_t sensorList[10] =
  {
    BSEC_OUTPUT_RAW_TEMPERATURE,
    BSEC_OUTPUT_RAW_PRESSURE,
    BSEC_OUTPUT_RAW_HUMIDITY,
    BSEC_OUTPUT_RAW_GAS,
    BSEC_OUTPUT_IAQ,
    BSEC_OUTPUT_STATIC_IAQ,
    BSEC_OUTPUT_CO2_EQUIVALENT,
    BSEC_OUTPUT_BREATH_VOC_EQUIVALENT,
    BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_TEMPERATURE,
    BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_HUMIDITY,
  };
 
  iaqSensor.updateSubscription(sensorList, 10, BSEC_SAMPLE_RATE_LP);
  checkIaqSensorStatus();
 
  // Print the header
  output = "Timestamp [ms], raw temperature [°C], pressure [hPa], raw relative humidity [%], gas [Ohm], IAQ, IAQ accuracy, temperature [°C], relative humidity [%], Static IAQ, CO2 equivalent, breath VOC equivalent";
  Serial.println(output);
}

void loop(void)
{
  unsigned long time_trigger = millis();
  if (iaqSensor.run()) // If new data is available
  {
    output = String(time_trigger);
    output += ", " + String(iaqSensor.rawTemperature);
    output += ", " + String(iaqSensor.pressure);
    output += ", " + String(iaqSensor.rawHumidity);
    output += ", " + String(iaqSensor.gasResistance);
    output += ", " + String(iaqSensor.iaq);
    output += ", " + String(iaqSensor.iaqAccuracy);
    output += ", " + String(iaqSensor.temperature);
    output += ", " + String(iaqSensor.humidity);
    output += ", " + String(iaqSensor.staticIaq);
    output += ", " + String(iaqSensor.co2Equivalent);
    output += ", " + String(iaqSensor.breathVocEquivalent);
    Serial.println(output);
 
    Serial.print("Presión: ");
    Serial.print((iaqSensor.pressure/100));
    Serial.println(" hPa");
 
    Serial.print("Temperatura: ");
    Serial.print(iaqSensor.temperature);
    Serial.println(" *C");
 
    Serial.print("Humedad: ");
    Serial.print(iaqSensor.humidity);
    Serial.println(" %");
 
    Serial.print("ICA: ");
    Serial.print(iaqSensor.iaq);
    Serial.println(" PPM");
 
    Serial.print("CO2 Equivalente: ");
    Serial.print(iaqSensor.co2Equivalent);
    Serial.println(" PPM");
 
    Serial.print("COV Respiración Equivalente: ");
    Serial.print(iaqSensor.breathVocEquivalent);
    Serial.println(" PPM");
    Serial.println();
  
    Blynk.run();
    Blynk.virtualWrite(V1, (iaqSensor.pressure)/100);        // For Pressure
    Blynk.virtualWrite(V2, iaqSensor.temperature);            // For Temperature
    Blynk.virtualWrite(V3, iaqSensor.humidity);               // For Humidity
    Blynk.virtualWrite(V4, iaqSensor.iaq);                    //For Index of Air Quality
    Blynk.virtualWrite(V5, iaqSensor.co2Equivalent);          // For CO2
    Blynk.virtualWrite(V6, iaqSensor.breathVocEquivalent);    // For Breath VoC
    if (iaqSensor.co2Equivalent > 600 && iaqSensor.co2Equivalent < 800){
      Blynk.logEvent("aviso_co2");
    }    
    if (iaqSensor.co2Equivalent > 800){
      Blynk.logEvent("alerta_co2");
    }   
  }
  else
  {
    checkIaqSensorStatus();
  }
}

void checkIaqSensorStatus(void)
{
  if (iaqSensor.status != BSEC_OK)
  {
    if (iaqSensor.status < BSEC_OK)
    {
      output = "BSEC error code : " + String(iaqSensor.status);
      Serial.println(output);
      for (;;)
        errLeds(); /* Halt in case of failure */
    }
    else
    {
      output = "BSEC warning code : " + String(iaqSensor.status);
      Serial.println(output);
    }
  }
 
  if (iaqSensor.bme680Status != BME680_OK)
  {
    if (iaqSensor.bme680Status < BME680_OK)
    {
      output = "BME680 error code : " + String(iaqSensor.bme680Status);
      Serial.println(output);
      for (;;)
        errLeds(); /* Halt in case of failure */
    }
    else
    {
      output = "BME680 warning code : " + String(iaqSensor.bme680Status);
      Serial.println(output);
    }
  }
}
 
void errLeds(void)
{
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(100);
  digitalWrite(LED_BUILTIN, LOW);
  delay(100);
}
