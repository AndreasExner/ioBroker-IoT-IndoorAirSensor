/*

  Indoor air sensor for iobroker IoT Framework

  Version: F5_1.3 (release)
  Date: 2020-11-30

  This sketch has several prerquisites discribed in the documentation of the repository:
  https://github.com/AndreasExner/ioBroker-IoT-IndoorAirSensor
  
  This sketch is based on my ioBroker IoT Framework V5
  https://github.com/AndreasExner/ioBroker-IoT-Framework


  MIT License
  
  Copyright (c) 2020 Andreas Exner
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
  
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/

//+++++++++++++++++++++++++++++++++++++++++ enable sensor specific functions +++++++++++++++++++++++++++++++++++++++++++++++++

#define AEX_iobroker_IoT_Framework //generic functions DO NOT CHANGE

// uncomment required sections

#define BME280_active
#define BME680_active
#define SCD30_active
//#define WindSensor_active

//+++++++++++++++++++++++++++++++++++++++++ generic device section +++++++++++++++++++++++++++++++++++++++++++++++++

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

// device settings - change settings to match your requirements

const char* ssid     = "<ssid>"; // Wifi SSID
const char* password = "<password>"; //Wifi password

String SensorID = "04"; //predefinded sensor ID, DEV by default to prevent overwriting productive data

int interval = 10;  // waiting time for the first masurement and fallback on error reading interval from iobroker

/*
 * The BSEC library sets the pace for the loop duration -> BSEC_LP ~ 3000 ms per request
 * For a transmission interval of 5 minutes configure an interval of 100 (*3 = 300 seconds = 5 minutes)
*/

bool DevMode = true; //enable DEV mode on boot (do not change)
bool debug = false; //debug to serial monitor
bool led = true; //enable external status LED on boot
bool sensor_active = false; // dectivate sensor(s) on boot (do not change)

/*
 * build base URL's
 * Change IP/FQND and path to match your environment
 */

String baseURL_DEVICE_GET = "http://192.168.1.240:8087/getPlainValue/0_userdata.0.IoT-Devices." + SensorID + ".";
String baseURL_DEVICE_SET = "http://192.168.1.240:8087/set/0_userdata.0.IoT-Devices." + SensorID + ".";

// end of device settings - don not change anything below the line until required

// define generic device URL's

String URL_IP = baseURL_DEVICE_SET + "SensorIP?value=";
String URL_RST = baseURL_DEVICE_SET + "Reset?value=";
String URL_LED = baseURL_DEVICE_GET + "LED";
String URL_MAC = baseURL_DEVICE_SET + "MAC?value=";
String URL_RSSI = baseURL_DEVICE_SET + "RSSI?value=";
String URL_DevMode = baseURL_DEVICE_GET + "DevMode";
String URL_ErrorLog = baseURL_DEVICE_SET + "ErrorLog?value=";
String URL_sensor_active = baseURL_DEVICE_GET + "SensorActive";

String baseURL_DATA_GET, baseURL_DATA_SET; // URL's for data 
String URL_SID, URL_INT; // URL's for sensor ID and interval

// other definitions

#define LED D4 // gpio pin for external status LED
void(* HWReset) (void) = 0; // define reset function DO NOT CHANGE
int counter = interval;  // countdown for next interval

//+++++++++++++++++++++++++++++++++++++++++ BME680 section +++++++++++++++++++++++++++++++++++++++++++++++++++++++

#include <EEPROM.h>
#include "bsec.h"

bool BME680_activated = false;

/* Configure the BSEC library with information about the sensor
    18v/33v = Voltage at Vdd. 1.8V or 3.3V
    3s/300s = BSEC operating mode, BSEC_SAMPLE_RATE_LP or BSEC_SAMPLE_RATE_ULP
    4d/28d = Operating age of the sensor in days
    generic_18v_3s_4d
    generic_18v_3s_28d
    generic_18v_300s_4d
    generic_18v_300s_28d
    generic_33v_3s_4d
    generic_33v_3s_28d
    generic_33v_300s_4d
    generic_33v_300s_28d
*/
const uint8_t bsec_config_iaq[] = {
#include "config/generic_33v_3s_4d/bsec_iaq.txt"
};

#define STATE_SAVE_PERIOD  UINT32_C(360 * 60 * 1000) // 360 minutes - 4 times a day


// Create an object of the class Bsec
Bsec iaqSensor;
uint8_t bsecState[BSEC_MAX_STATE_BLOB_SIZE] = {0};
uint16_t stateUpdateCounter = 0;

int iaSAhistory = 0;

String iaqD, iaDA, iaqS, iaSA, VOCe, temp, humi;
String URL_iaqD, URL_iaDA, URL_iaqS, URL_iaSA, URL_VOCe;
String URL_BME680_updateState, URL_BME680_loadState, URL_BME680_eraseEEPROM, URL_BME680_reset_get, URL_BME680_reset_set;

//+++++++++++++++++++++++++++++++++++++++++ BME280 section +++++++++++++++++++++++++++++++++++++++++++++++++++++++

#include <Adafruit_BME280.h>

bool BME280_activated = false;

Adafruit_BME280 bme;

String bme280_temp, bme280_humi, bme280_airp, bme280_alti;
String URL_temp, URL_humi, URL_airp;
float bme280_pressure;
int pressure_sl;

String URL_PRESL = "http://192.168.67.240:8087/getPlainValue/daswetter.0.NextHours.Location_1.Day_1.pressure_value";  //reference for pressure at sea level for altitude

//+++++++++++++++++++++++++++++++++++++++++ SCD30 section +++++++++++++++++++++++++++++++++++++++++++++++++++++++

#include <Wire.h>
#include "SparkFun_SCD30_Arduino_Library.h"

bool SCD30_activated = false;

SCD30 airSensor;
int scd30_altitude = 169; //sensors's altitude (m over sealevel)
int scd30_interval = 3; // intervall in sec.
int scd30_offset = 2; // temperature offset

String scd30_autoCalHistory = "false";
String scd30_co2, scd30_humi, scd30_temp;
String URL_co2, URL_SCD30_autoCal_get;

//######################################### setup ##############################################################

void setup(void) {

  Serial.begin(115200);  
  delay(1000);
  
  pinMode(LED, OUTPUT);

  connect_wifi();
  get_wifi_state();

// initial communication with iobroker on boot

  bool debug_state = debug;  //debug output during setup
  debug = true;

  get_dynamic_config();
  build_urls();
  send_ip();
  send_sid();
  send_rst();
     
  debug = debug_state;

// setup sensors

  if (sensor_active) {BME680_setup();}
  if (sensor_active) {BME280_setup();}
  if (sensor_active) {SCD30_setup();}

}

//######################################### specific device functions #######################################################

void build_urls() {

  URL_SID = baseURL_DATA_SET + "SensorID?value=" + SensorID;
  URL_INT = baseURL_DATA_GET + "Interval";
  
  URL_BME680_updateState = baseURL_DATA_SET + "BME680_updateState?value=";
  URL_BME680_loadState = baseURL_DATA_SET + "BME680_loadState?value=";
  URL_BME680_eraseEEPROM = baseURL_DATA_SET + "BME680_eraseEEPROM?value=";
  URL_BME680_reset_get = baseURL_DATA_GET + "BME680_reset";
  URL_BME680_reset_set = baseURL_DATA_SET + "BME680_reset?value=";

  URL_iaqD = baseURL_DATA_SET + "iaqD?value=";
  URL_iaDA = baseURL_DATA_SET + "iaDA?value=";
  URL_iaqS = baseURL_DATA_SET + "iaqS?value=";
  URL_iaSA = baseURL_DATA_SET + "iaSA?value=";
  URL_VOCe = baseURL_DATA_SET + "VOCe?value=";

  URL_temp = baseURL_DATA_SET + "temp?value=";
  URL_humi = baseURL_DATA_SET + "humi?value=";
  URL_airp = baseURL_DATA_SET + "airp?value="; 

  URL_co2 = baseURL_DATA_SET + "co2?value=";
  URL_SCD30_autoCal_get = baseURL_DATA_GET + "SCD30_autoCal";
}

void send_data() {

  Serial.println("send data to iobroker");

  HTTPClient http;
  
  String sendURL;

  sendURL = URL_iaDA + iaDA;
  http.begin(sendURL);
  http.GET();
  
  sendURL = URL_iaSA + iaSA;
  http.begin(sendURL);
  http.GET();

  sendURL = URL_co2 + scd30_co2;
  http.begin(sendURL);
  http.GET();

  sendURL = URL_humi + bme280_humi;
  http.begin(sendURL);
  http.GET();

  sendURL = URL_temp + bme280_temp;
  http.begin(sendURL);
  http.GET();

  sendURL = URL_airp + bme280_airp;
  http.begin(sendURL);
  http.GET();

  sendURL = URL_iaqD + iaqD;
  http.begin(sendURL);
  http.GET();

  sendURL = URL_iaqS + iaqS;
  http.begin(sendURL);
  http.GET();

  sendURL = URL_VOCe + VOCe;
  http.begin(sendURL);
  http.GET();
  
  http.end();
} 


//####################################################################
// Loop
  
void loop(void) {

  /* run BME680 sample and wait for new data
   *  
   * The BSEC library sets the pace for the loop interval -> LP = 3000 ms pre run
   * results in round about 0,3Hz. For a transmission interval of 5 minutes configure 
   * an interval of 100 (x3 = 300 seconds)
   * EVERY command has to be included in the if-clause!!
   */
  
  if (iaqSensor.run()) { 

    // read data and serial output

    if (sensor_active && BME680_activated) {BME680_get_data();}
    if (sensor_active && BME280_activated) {BME280_get_data();}
    if (sensor_active && SCD30_activated) {SCD30_get_data();}

    if (sensor_active && BME680_activated) {BME680_serial_output();}
    if (sensor_active && BME280_activated) {BME280_serial_output();}
    if (sensor_active && SCD30_activated) {SCD30_serial_output();}

    // send data to iobroker ever n-th BME680 run
      
    if (counter == 0) { 

      get_wifi_state();
      send_ip();

      get_dynamic_config();
      build_urls();
      
      if (sensor_active && BME280_activated && BME680_activated  && SCD30_activated) {send_data();}
      if (sensor_active && BME680_activated) {BME680_reset();}

      if (sensor_active && BME680_activated == false) {BME680_setup();}
      if (sensor_active && BME280_activated == false) {BME280_setup();}
      if (sensor_active && SCD30_activated == false) {SCD30_setup();}

      if (sensor_active && BME280_activated) {BME280_get_sealevel_pressure();}
      if (sensor_active && SCD30_activated) {SCD30_AutoCal();}

      get_interval();
      counter = interval;
      
    } 
    else {
      counter--;  
      Serial.println(counter);
    }

    // Blink LED
  
    if (led && (counter % 2)) {
      digitalWrite(LED, LOW);         
    }
    else {
      digitalWrite(LED, HIGH);
    }
  
    // check sensor if BME680 run fails
  
  } else {
    if (sensor_active && BME680_activated) {
      BME680_checkIaqSensorStatus();
    }
  }
}
