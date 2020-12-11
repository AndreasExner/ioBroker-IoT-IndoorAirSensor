# ioBroker-IoT-IndoorAirSensor

**Important: This project based on my ioBroker IoT Framework V5**

[AndreasExner/ioBroker-IoT-Framework: ioBroker IoT Framework (based on NodeMCU ESP8266) (github.com)](https://github.com/AndreasExner/ioBroker-IoT-Framework)

**Please refer to this documentation for the the basic setup and configuration.**

Required Version: 5.3.0 (or higher)


## Description

#### Introduction

I've tested some all-in-one sensors for climate and air quality monitoring. But most of them are specialized and calibrated on a single component. Other components are derived and more or less useless. For example, the C02 equivalent is derived from the IAQ measurement. Every time the sensor recognizes VOC's in the air, the CO2e level raises in the same way. What is not the fact. In addition, the BME680 sensor is a heat source. The own measured temperature and humidity results are significantly influenced by that. The same the SCD30. It is a very precise NDIR CO2 sensor with ASC, but the temperature and humidity measurement is inaccurate, due to a heat source. 

In the end, these three sensors are able to provide a very precise collection of data:

1. BME280: temperature, humidity and ambient pressure
2. BME680: IAQ and VOCe
3. SCD30: CO2



#### ePaper Display

I've decided to add an ePaper (Waveshare 1.54inch e-Paper Module) as local display to read the values right from the device. The ePaper needs no backlight and don't produce irritating light in the room. For example, in the bedroom. 

The ePaper library is not yet integrated into the IoT Framework, except the "LastUpdate" function. The code varies very much, depending on the display type/size and the content. 

Please use the documentation of the repository for more information: [ZinggJM/GxEPD: A simple E-Paper display library with common base class and separate IO class for Arduino. (github.com)](https://github.com/ZinggJM/GxEPD)



#### Wiring

All sensors are connected with via I2C. Ensure an unique address for each sensor. 

![](https://github.com/AndreasExner/ioBroker-IoT-IndoorAirSensor/blob/main/IndoorAirSensor_Steckplatine.jpg?raw=true)



## History

F5_2.0 (release) 2020-12-10

- added ePaper Display
- minor bug fixes and cleanups

F5_1.3 (release) 2020-12-02



#### Tested environment

- Software
  - Arduino IDE 1.8.13 (Windows)
  - ESP8266 hardware package 2.7.4
  - Adafruit_BME280_Library version 2.1.2
  - BSEC-Arduino-library 1.6.1480
  - SparkFun_SCD30_Arduino_Library release 9
    **Important:** This sketch requires two additional functions: https://github.com/sparkfun/SparkFun_SCD30_Arduino_Library/pulls
  - GxEPD library 3.1.0
- Hardware
  - NodeMCU Lolin V3 (ESP8266MOD 12-F)
  - GY-BME280
  - CJMCU-680 BME680
  - Sensirion SCD30
  - Waveshare 1.54inch e-Paper Module



## Prerequisites

* You need a running Arduino IDE and at least basic knowledge how it works. 
* You also need a running ioBroker instance and a little more than basic knowledge on that stuff.
* You need a REST API in your ioBroker setup
* You need to **secure** the REST API (for example, expose the required data only and hide any confidential and secret stuff)
* You need a userdata section in your ioBroker objects
* You should know how to work with IoT devices and electronics
* You need to read this first: [AndreasExner/ioBroker-IoT-Framework: ioBroker IoT Framework (based on NodeMCU ESP8266) (github.com)](https://github.com/AndreasExner/ioBroker-IoT-Framework)



## Setup

- Create a folder in your Arduino library folder
- Copy the primary sketch (e.g. IndoorAirSensor.ino) and the extension file (AEX_iobroker_IoT_Framework.ino) into the folder
- Open the primary sketch (e.g. IndoorAirSensor.ino) 
- **Install required libraries into your Arduino IDE**
- Create (import) the datapoints in iobroker
  - 0_userdata.0.IoT-Devices.04.json (generic device configuration and monitoring)
  - 0_userdata.0.IoT.IndoorAirSensor.json (specific device configuration and data, production)
  - 0_userdata.0.IoT-Dev.IndoorAirSensor.json (specific device configuration and data, development, optional)
- Set values for datapoints (see iobroker datapoints)



## Configuration

#### Generic device section

```c++
// device settings - change settings to match your requirements

const char* ssid     = "<ssid>"; // Wifi SSID
const char* password = "<password>"; //Wifi password

String SensorID = "DEV"; //predefinded sensor ID, DEV by default to prevent overwriting productive data

int interval = 10;  // waiting time for the first masurement and fallback on error reading interval from iobroker

bool DevMode = true; //enable DEV mode on boot (do not change)
bool debug = true; //debug to serial monitor
bool led = true; //enable external status LED on boot
bool sensor_active = false; // dectivate sensor(s) on boot (do not change)
```

- Enter proper Wifi information
- The **`SensorID`** is part of the URL's and important for the the iobroker communications
- **`Interval`** defines the delay between two data transmissions / measurements. This value is used initially after boot. The interval dynamically updates from iobroker
- The **`DevMode`** switch prevents the device from sending data into your productive datapoints. It is enabled by default and can be overwritten dynamically from iobroker
- **`debug`** enables a more detailed serial output
- **`led`** enables the onboard led (status)
- The **`sensor_active`** switch disables the loading of hardware specific code on startup. This is very useful to test a sketch on the bread board without the connected hardware. It is disabled by default and gets dynamically enabled from the iobrocker during boot, as long as nothing else is configured.



#### Base URL's

```c++
/*
 * build base URL's
 * Change IP/FQND and path to match your environment
 */

String baseURL_DEVICE_GET = "http://192.168.1.240:8087/getPlainValue/0_userdata.0.IoT-Devices." + SensorID + ".";
String baseURL_DEVICE_SET = "http://192.168.1.240:8087/set/0_userdata.0.IoT-Devices." + SensorID + ".";
```

The base url's, one for read and one to write data, are pointing to your iobroker datapoints in the devices section. The SensorID will be added to complete the path. 



#### BME280 section

```c++
String URL_PRESL = "http://192.168.1.240:8087/getPlainValue/daswetter.0.NextHours.Location_1.Day_1.pressure_value";
```

The BME280 requires the actual see level air pressure to calculate the altitude (optional). The sketch gets the information from the iobroker adapter.



## iobroker datapoints

#### Devices section

The default path for the devices root folder is: **`0_userdata.0.IoT-Devices`**. When the path is changed, it has to be changed in the sketch as well.

**It is mandatory to setup the following datapoints prior the first device boot:**

- **`DevMode`** If true, the baseURL's for the IoT-Dev section are used to prevent overwriting your production data. Also see generic device section.

- **`LED`** Controls the status LED. Also see generic device section.

- **`SensorActive`** Controls the hardware sensors. Also see generic device section.

- **`SensorName`** Easy to understand name for the sensor. Not used in the code.

- **`baseURL_GET_DEV`** points to the IoT-Dev datapoints in iobroker

  - ```
    http://192.168.1.240:8087/getPlainValue/0_userdata.0.IoT-Dev.IndorAirSensor.
    ```

- **`baseURL_SET_DEV`** points to the IoT-Dev datapoints in iobroker

  - ```
    http://192.168.1.240:8087/set/0_userdata.0.IoT-Dev.IndorAirSensor.
    ```

- **`baseURL_GET_PROD`** points to the IoT datapoints in iobroker

  - ```
    http://192.168.1.240:8087/getPlainValue/0_userdata.0.IoT.IndorAirSensor.
    ```

- **`baseURL_SET_PROD`** points to the IoT datapoints in iobroker

  - ```
    http://192.168.1.240:8087/set/0_userdata.0.IoT.IndorAirSensor.
    ```



#### Specific device configuration and data

Depending on the **`DevMode`**, the device reads config and writes data into different datapoint sections:

- Development root folder: **`0_userdata.0.IoT-Dev`**
- Production root folder: **`0_userdata.0.IoT`**

It is recommended to keep the datapoints in both sections identical to avoid errors when changing the **`DevMode`**. The values can be different. Setup these datapoints before boot up the device:

- **`Interval`** [100] Defines the delay between two data transmissions / measurements. **Important:** due to the behavior of the BSEC library, the loop takes more or less 3000ms. E.g, an interval of 100 results in a delay of 300 seconds (5 minutes).  
- **`BME680_reset`** [false] Triggers a reset (EraseEEPROM) of the BME680 sensor when true. Flips back to false when the reset was done 
- **`SCD30_autoCal`** [true] Enables the SCD30 ASC

These datapoints are for output only:

- **`BME680_eraseEEPROM`** Timestamp for the last eraseEEPROM event
- **`BME680_loadState`** Timestamp for the last loadState event
- **`BME680_updateStatet`** Timestamp for the last updateState event
- **`SensorID`** Sensor device ID
- **`temp, humi, airp`** Temperature in °C, humidity in % and ambient air pressure in mbar (BME280)
- **`iaqS, iaqD`** IAQ index static, IAQ index dynamic (BME680)
- **`iaSA, iaDA`** IAQ accuracy static, IAQ accuracy dynamic (BME680)
- **`VOCe`** VOC equivalent, in mg/m³ (BME680)
- **`co2`** CO2 concentration in ppm (SCD30)

This datapoint is used to provide a time string of the last update to the device

- **`LastUpdate`** LastUpdate String



## LastUpdate string

Because the device has no real time clock, it is required to provide a timestamp string for the last update event. This is important, because the ePaper display does not change or goes down, even when the device is unplugged. That can lead into confusion and totally wrong values on the display. The timestamp string allows the user to validate that the device is working well.

This iobroker script is used to provide this string, triggered by an update of the IP address datapoint:

```javascript
on({id: "0_userdata.0.IoT-Devices.04.SensorIP", change: "any"}, function (obj) {
  var value = obj.state.val;
  var oldValue = obj.oldState.val;
  setState("0_userdata.0.IoT.IndoorAirSensor02.LastUpdate", ([formatDate(new Date(), "DD.MM."),' - ',formatDate(new Date(), "hh:mm")].join('')), true);
});
```



## How it works

#### Boot phase / setup

- Connect Wifi
- Get Wifi State
- Get the dynamic configuration from iobroker (generic devices section)
- Build specific device URL's (based on the dynamic configuration)
- Send devices IP address
- Send device ID
- Send information about the last restart / reset
- Run's setup for active sensors



#### Loop

**Important:** due to the behavior of the BSEC library, it is recommended to wait for the BME680 data. It is not finally confirmed, but I've seen strange values from the sensor if not. Since the BSEC library defines a sample rate of 0,33 Hz in LP mode, the loop needs round about 3 seconds to proceed. The status led blinks with the half frequency.

Every n-th tick, defined by the **`Interval`**,  the following sequence will proceed:

- Loop (0,33 Hz) -> BME680 Data ready
  - Get data of all sensors
  - Serial output all sensor data
  - Interval reached
    - Get Wifi State (try reconnect first, then reset if not connected)
    - Send devices IP address
    - Get the dynamic configuration from iobroker (generic devices section)
    - Build specific device URL's (based on the dynamic configuration)
    - Send data to iobroker
    - Get LastUpdate
    - Show data on ePaper display
    - Check reset flag for BM680 and execute reset if requested
    - Run setup for inactive sensors (if activated now)
    - Update the sealevel pressure as reference for the altitude for BME280 from iobroker (internet source)
    - Check / update SCD30 ASC
    - Get the new interval (specific device section)
  - LED Blink
- BME680 Data not ready
  - Check BME680 sensor


## Appendix

#### BME680 / BSEC

The Bosch BSEC library uses precompiled libraries. You need to make some changes to the ESP8266 plattform.txt file to allow and include precompiled libraries:

1. find the plattform.txt file for your hardware package. The default path on a Windows PC should be for example: 

   ```
   C:\Users\<username>\AppData\Local\Arduino15\packages\esp8266\hardware\esp8266\2.7.4
   ```

   

2. Add the line "compiler.libraries.ldflags=" (block starts at line #82):

   ```c++
   # These can be overridden in platform.local.txt
   compiler.c.extra_flags=
   compiler.c.elf.extra_flags=
   compiler.S.extra_flags=
   compiler.cpp.extra_flags=
   compiler.ar.extra_flags=
   compiler.objcopy.eep.extra_flags=
   compiler.elf2hex.extra_flags=
   #### added for BSEC
   compiler.libraries.ldflags=
   ```

   

3. Change the line "Combine gc-sections, archives, and objects" (starts at line #113) and add "{compiler.libraries.ldflags}" directive at the suggested position:

   ```c++
   ## Combine gc-sections, archives, and objects
   # recipe.c.combine.pattern="{compiler.path}{compiler.c.elf.cmd}" {build.exception_flags} -Wl,-Map "-Wl,{build.path}/{build.project_name}.map" {compiler.c.elf.flags} {compiler.c.elf.extra_flags} -o "{build.path}/{build.project_name}.elf" -Wl,--start-group {object_files} "{archive_file_path}" {compiler.c.elf.libs} -Wl,--end-group  "-L{build.path}"
   #### changed for BSEC
   recipe.c.combine.pattern="{compiler.path}{compiler.c.elf.cmd}" {build.exception_flags} -Wl,-Map "-Wl,{build.path}/{build.project_name}.map" {compiler.c.elf.flags} {compiler.c.elf.extra_flags} -o "{build.path}/{build.project_name}.elf" -Wl,--start-group {object_files} "{archive_file_path}" {compiler.c.elf.libs} {compiler.libraries.ldflags} -Wl,--end-group  "-L{build.path}"
   
   ```
