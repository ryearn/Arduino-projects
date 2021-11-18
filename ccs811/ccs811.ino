/*
  ccs811basic.ino - Demo sketch printing results of the CCS811 digital gas sensor for monitoring indoor air quality from ams.
  Created by Maarten Pennings 2017 Dec 11
*/


#include <Wire.h>    // I2C library
#include "ccs811.h"  // CCS811 library


// Wiring for ESP8266 NodeMCU boards: VDD to 3V3, GND to GND, SDA to D2, SCL to D1, nWAKE to D3 (or GND)
CCS811 ccs811(D3); // nWAKE on D3

// Wiring for Nano: VDD to 3v3, GND to GND, SDA to A4, SCL to A5, nWAKE to 13
//CCS811 ccs811(13); 

// nWAKE not controlled via Arduino host, so connect CCS811.nWAKE to GND
//CCS811 ccs811; 

// Wiring for ESP32 NodeMCU boards: VDD to 3V3, GND to GND, SDA to 21, SCL to 22, nWAKE to D3 (or GND)
//CCS811 ccs811(23); // nWAKE on 23

// dht11 on d7
#include "DHTesp.h" // Click here to get the library: http://librarymanager/All#DHTesp
DHTesp dht;


#include <AverageValue.h>
AverageValue<uint16_t> eco2_average(6);
AverageValue<uint16_t> etvoc_average(6);

AverageValue<float> dht_temp_average(6);
AverageValue<float> dht_hum_average(6);


void setup() {
  // Enable serial
  Serial.begin(115200);
  //Serial.println("");
  Serial.println("# setup: Starting CCS811 basic demo");
  Serial.print("# setup: ccs811 lib  version: "); Serial.println(CCS811_VERSION);

  // Enable I2C
  Wire.begin(); 
  
  // Enable CCS811
  ccs811.set_i2cdelay(50); // Needed for ESP8266 because it doesn't handle I2C clock stretch correctly
  bool ok= ccs811.begin();
  if( !ok ) Serial.println("# setup: CCS811 begin FAILED");

  // Print CCS811 versions
  Serial.print("# setup: hardware    version: "); Serial.println(ccs811.hardware_version(),HEX);
  Serial.print("# setup: bootloader  version: "); Serial.println(ccs811.bootloader_version(),HEX);
  Serial.print("# setup: application version: "); Serial.println(ccs811.application_version(),HEX);
  
  // Start measuring
  ok= ccs811.start(CCS811_MODE_1SEC);
  if( !ok ) Serial.println("# setup: CCS811 start FAILED");

  dht.setup(13, DHTesp::DHT11); // Connect DHT sensor to D7 - GPIO 13

}


void loop() {

  float temperature = dht.getTemperature();
  Serial.print("dht_temp="); Serial.print(temperature); Serial.print(" ");
  dht_temp_average.push(temperature);
  Serial.print("dht_temp_avg="); Serial.print(dht_temp_average.average()); Serial.print(" ");

  float humidity = dht.getHumidity();
  Serial.print("dht_hum="); Serial.print(humidity); Serial.print(" ");
  dht_hum_average.push(humidity);
  Serial.print("dht_hum_avg="); Serial.print(dht_hum_average.average()); Serial.print(" ");

  ccs811.set_envdata_Celsius_percRH( dht_temp_average.average(), dht_hum_average.average() );

  // Read
  uint16_t eco2, etvoc, errstat, raw;
  ccs811.read(&eco2,&etvoc,&errstat,&raw); 
  
  // Print measurement results based on status
  if( errstat==CCS811_ERRSTAT_OK ) { 

    eco2_average.push(eco2);
    Serial.print("eco2_avg="); Serial.print(eco2_average.average()); Serial.print(" ");

    etvoc_average.push(etvoc);
    Serial.print("etvoc_avg="); Serial.print(etvoc_average.average()); Serial.print(" ");
    
    //Serial.print("CCS811: ");
    Serial.print("eco2=");  Serial.print(eco2);//     Serial.print(" ppm  ");
    Serial.print(" ");
    Serial.print("etvoc="); Serial.print(etvoc); //   Serial.print(" ppb  ");
    //Serial.print("raw6=");  Serial.print(raw/1024); Serial.print(" uA  "); 
    //Serial.print("raw10="); Serial.print(raw%1024); Serial.print(" ADC  ");
    //Serial.print("R="); Serial.print((1650*1000L/1023)*(raw%1024)/(raw/1024)); Serial.print(" ohm");
    Serial.println();
  } else if( errstat==CCS811_ERRSTAT_OK_NODATA ) {
    Serial.println("# CCS811: waiting for (new) data");
  } else if( errstat & CCS811_ERRSTAT_I2CFAIL ) { 
    Serial.println("# CCS811: I2C error");
  } else {
    Serial.print("# CCS811: errstat="); Serial.print(errstat,HEX); 
    Serial.print("="); Serial.println( ccs811.errstat_str(errstat) ); 
  }
  
  // Wait
  delay(1000); 
}
