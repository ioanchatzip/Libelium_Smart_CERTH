/* Libraries */
#include <WaspLoRaWAN.h>
#include <WaspSensorSW.h>
#include <lora_send.h>
#include <lora_join.h>

/** Socket Selection for the Sensor Probe */
uint8_t socket = SOCKET0;

/** Device parameters for Back-End registration */
char device_eui[] = "72c465dcf469a174";
char app_eui[] = "72c465dcf469a174";
char app_key[] = "66c203b5c8149813ef44cb88c5bd88be";

/** Define port to use in Back-End: from 1 to 223 */
uint8_t port = 10;

/** Define data payload to send (maximum is up to data rate) */
uint8_t bytes[11] = {0};

/** Variables */
int battery_level = 0;
float pH_voltage;
float temperature;
float pH_value;
int16_t temperature_send;
int pH_value_send;
uint8_t size_of_buffer = sizeof(bytes) / sizeof(bytes[0]); /* or = 11*/
uint8_t adr_flag = 0; /**< Adaptive Data Rate indicator for lora_join() */

/* Calibration values @ 23.7 Degrees Celcius */
#define CALIBRATION_POINT_10  1.985 /**< pH - 10, 1.985 Volts */
#define CALIBRATION_POINT_7   2.070 /**< pH - 7, 2.070 Volts */
#define CALIBRATION_POINT_4   2.227 /**< pH - 4, 2.227 Volts */
#define CALIBRATION_TEMPERATURE 23.7

/* Classes for the sensors */
pHClass pHSensor;
pt1000Class temperatureSensor;

void setup() 
{
    USB.ON();
    USB.println(F("LoRaWAN example - Send Confirmed packets (ACK)\n"));
    USB.println(F("pH example for Smart Water..."));
    USB.println(F("------------------------------------"));
    USB.println(F("Module configuration"));
    USB.println(F("------------------------------------\n"));

    /* Store the calibration values */
    pHSensor.setCalibrationPoints(CALIBRATION_POINT_10,
                                  CALIBRATION_POINT_7, 
                                  CALIBRATION_POINT_4,
                                  CALIBRATION_TEMPERATURE);
    /* Turn on the sensor board */
    Water.ON(); 
        
    lora_join(device_eui, app_eui, app_key, socket, adr_flag);
}

void loop() 
{  
    /* Read and Print Data from Sensors and Battery level */
    pH_voltage = pHSensor.readpH();
    temperature = temperatureSensor.readTemperature();
    
    /* Convert the value read with the information obtained
     * in calibration */
    pH_value = pHSensor.pHConversion(pH_voltage, temperature);

    USB.print(F("pH Value: "));
    USB.println(pH_value);
    USB.print(F("Temperature Value: "));
    USB.println(temperature);

    battery_level = PWR.getBatteryLevel();
    USB.print(F("Battery Level: "));
    USB.print(battery_level);
    USB.println(F(" %"));

    /* Decoding values to base64 for send preparation */
    temperature_send = (int16_t)(100 * temperature);
    pH_value_send = (int)(100 * pH_value);

    /* Preparing the bytes vector to send.
     * Store 2 identification vector posistions for each variable. */
    bytes[0] = 0x07;
    bytes[1] = 0x02;

    /* 8 positions of binary right shift@ Memory address 255
     * (0xFF in Hexadecimal) */
    bytes[2] = (temperature_send >> 8) & 0xFF; 

    /* Store the variable unchanged */
    bytes[3] =  temperature_send & 0xFF;

    bytes[4] = 0x08;
    bytes[5] = 0x02;
    bytes[6] = (pH_value_send >> 8) & 0xFF;
    bytes[7] =  pH_value_send & 0xFF;
    
    bytes[8] = 0x06;
    bytes[9] = 0x01;
    bytes[10] = battery_level & 0xFF;

    lora_send(device_eui, app_eui, app_key, size_of_buffer,
            port, socket, bytes);
}