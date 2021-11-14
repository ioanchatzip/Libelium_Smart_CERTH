/* Libraries and Definitions */
#include <WaspLoRaWAN.h>
#include <WaspSensorGas_v30.h>
#include <lora_send.h>
#include <lora_join.h>
#define MINUTESTOSEND 28 /**< Time in minutes */
#define CALIBRATION_POINTS 3 /**< 3 Calibration points */

/** Device parameters for Back-End registration */
uint8_t socket = SOCKET0;

/* Device parameters for Back-End registration */
char device_eui[] = "72c465dcdb8b2afd";
char app_eui[] = "72c465dcdb8b2afd";
char app_key[] = "c1e90d187fb93834e60ba43c26fb9282";

/** Define port to use in Back-End: from 1 to 223 */
uint8_t port = 10;

/** Define data payload to send (maximum is up to data rate) */
char bytes[33] = {0};

/** General Variables */
int battery_level = 0;
float luxes;
uint8_t error;
uint8_t i = 0;
int luxes_send = 0;
uint8_t size_of_buffer = sizeof(bytes) / sizeof(bytes[0]); /* or = 33 */
uint8_t adr_flag = 0; /**< Adaptive Data Rate indicator for lora_join() */

/** Sensor Variables */
/* for CO Data */
float co_voltage;
float co_resistance;
float co_particles;
int co_send = 0;

/* for NO2 Data */
float no2_voltage;
float no2_resistance;
float no2_particles;
int no2_send = 0;

/* for Air Pollutants Data (first set) */
float polutants_voltage;
float polutants_resistance;
float polutants_particles;
int polutants_send = 0;

/* for Air Pollutants Data (second set) */
float polutants2_voltage;
float polutants2_resistance;
float polutants2_particles;
int polutants2_send = 0;

/* Classes for the sensors */
/* CO Sensor must be connected physically in SOCKET_4 */
COSensorClass COSensor;
/* NO2 Sensor must be connected physically in SOCKET_3 */
NO2SensorClass NO2Sensor;
/* Air Pollutants Sensor first set -> SOCKET_6 and second set -> SOCKET_7 */
APSensorClass APPSensor(SOCKET_6);
APSensorClass APPSensor2(SOCKET_7);

/* Calibration Data */
/* CO Sensor */
/* Concentrations used in calibration process */
#define POINT1_PPM_CO 100.0 /**< Ro value at this concentration */
#define POINT2_PPM_CO 300.0
#define POINT3_PPM_CO 1000.0

/* Calibration resistances obtained during calibration process */
#define POINT1_RES_CO 20.3 /**< Ro Resistance at 100 ppm. Necessary value. */
#define POINT2_RES_CO 1.00
#define POINT3_RES_CO 0.25

/* CO Vectors for Concentrations and Resistances */
float concentrations_co[] = {POINT1_PPM_CO, POINT2_PPM_CO, POINT3_PPM_CO};
float resistances_co[] = {POINT1_RES_CO, POINT2_RES_CO, POINT3_RES_CO};

/* NO2 Sensor */
#define POINT1_PPM_NO2 10.0 /**< Normal concentration in the air */
#define POINT2_PPM_NO2 50.0
#define POINT3_PPM_NO2 100.0

/* Calibration voltages obtained during calibration process (in kOhms) */
#define POINT1_RES_NO2 2.00  /**< Rs at normal concentration in the air */
#define POINT2_RES_NO2 0.001
#define POINT3_RES_NO2 0.0002

/* NO2 Vectors for Concentrations and Voltages */
float concentrations_no2[] = {POINT1_PPM_NO2, POINT2_PPM_NO2, POINT3_PPM_NO2};
float voltages_no2[] = {POINT1_RES_NO2, POINT2_RES_NO2, POINT3_RES_NO2};

void setup()
{
    USB.ON();

    /* Air Pollutants Calibration (first set) */
    /* Concentrations used in calibration process (in ppm) */
    APPSensor.concentrations[POINT_1] = 10.0; /**< Ro value at this
                                               * concentration */
    APPSensor.concentrations[POINT_2] = 50.0;
    APPSensor.concentrations[POINT_3] = 100.0;

    /* Resistances obtained during calibration process (in kOhms) */
    APPSensor.values[POINT_1] = 8.00; /**< Ro Resistance at 100 ppm.
                                       * Necessary value. */
    APPSensor.values[POINT_2] = 0.665;
    APPSensor.values[POINT_3] = 0.300;

    /* Define the number of calibration points */
    APPSensor.numPoints = 3;
    /* Calculate the slope and the intersection
     * of the logarithmic function */
    APPSensor.setCalibrationPoints();
 
    /* Air Pollutants Calibration (second set) */
    /* Concentrations used in calibration process (in ppm)*/
    APPSensor2.concentrations[POINT_1] = 10.0; /**< Ro value at this
                                                * concentration */
    APPSensor2.concentrations[POINT_2] = 50.0;
    APPSensor2.concentrations[POINT_3] = 100.0;

    /* Resistances obtained during calibration process (in kOhms) */
    APPSensor2.values[POINT_1] = 4.00; /**< Ro Resistance at 100 ppm.
                                        * Necessary value. */
    APPSensor2.values[POINT_2] = 0.165;
    APPSensor2.values[POINT_3] = 0.0300;

    /* Define the number of calibration points */
    APPSensor2.numPoints = 3;
    /* Calculate the slope and the intersection
     * of the logarithmic function */
    APPSensor2.setCalibrationPoints();
  
    USB.println(F("LoRaWAN example - Send Confirmed packets (ACK)\n"));
    USB.println(F("GAS BOARD EXAMPLE..."));

    /* Store the calibration values */
    COSensor.setCalibrationPoints(resistances_co, concentrations_co,
                                  CALIBRATION_POINTS);
    NO2Sensor.setCalibrationPoints(voltages_no2, concentrations_no2,
                                   CALIBRATION_POINTS);
                                   
    lora_join(device_eui, app_eui, app_key, socket, adr_flag);
}

void loop()
{
    /* Read the sensors */
    Gases.ON();
    delay(100);
  
    /* CO Sensor */
    COSensor.ON();
    delay(1000);

    /* Read and print the CO sensor values
     * (Voltage, Resistance and Concentration) */
    co_voltage = COSensor.readVoltage();
    co_resistance = COSensor.readResistance();
    co_particles = COSensor.readConcentration();

    COSensor.OFF();
    USB.print(F("CO Sensor Voltage: "));
    USB.print(co_voltage);
    USB.print(F(" mV |"));
    USB.print(F(" CO Sensor Resistance: "));
    USB.print(co_resistance);
    USB.print(F(" Ohms |"));
    USB.print(F(" CO Sensor Concentration Estimated: "));
    USB.print(co_particles);
    USB.println(F(" ppm"));

    /* NO2 Sensor */
    NO2Sensor.ON();
    delay(30000);

    /* Read and print the NO2 sensor values
     * (Voltage, Resistance and Concentration) */
    no2_voltage = NO2Sensor.readVoltage();
    no2_resistance = NO2Sensor.readResistance();
    no2_particles = NO2Sensor.readConcentration();

    NO2Sensor.OFF();
    USB.print(F("NO2 Sensor Voltage: "));
    USB.print(no2_voltage);
    USB.print(F(" V |"));
    USB.print(F(" NO2 Sensor Resistance: "));
    USB.print(no2_resistance);
    USB.print(F(" Ohms |"));
    USB.print(F(" NO2 Sensor Concentration Estimated: "));
    USB.print(no2_particles);
    USB.println(F(" ppm"));

    /* Air Pollutants Sensor (first set) */
    APPSensor.ON();
    delay(30000);
  
    /* Read and print the first APP sensor values
     * (Voltage, Resistance and Concentration) */
    polutants_voltage = APPSensor.readVoltage();
    polutants_resistance = APPSensor.readResistance();
    polutants_particles = APPSensor.readConcentration();

    APPSensor.OFF();
    USB.print(F("1st Air Pollutants Sensor Voltage: "));
    USB.print(polutants_voltage);
    USB.print(F(" V |"));
    USB.print(F(" 1st Air Pollutants Sensor Resistance: "));
    USB.print(polutants_resistance);
    USB.print(F(" Ohms |"));
    USB.print(F(" 1st Air Pollutants Sensor Concentration Estimated: "));
    USB.print(polutants_particles);
    USB.println(F(" ppm"));

    /* Air Pollutants Sensor (second set) */
    APPSensor2.ON();
    delay(30000);

    /* Read and print the second APP sensor values
     * (Voltage, Resistance and Concentration) */
    polutants2_voltage = APPSensor2.readVoltage();
    polutants2_resistance = APPSensor2.readResistance();
    polutants2_particles = APPSensor2.readConcentration();

    APPSensor2.OFF();
    USB.print(F(" 2nd Air Pollutants Sensor Voltage: "));
    USB.print(polutants2_voltage);
    USB.print(F(" V |"));
    USB.print(F(" 2nd Air Pollutants Sensor Resistance: "));
    USB.print(polutants2_resistance);
    USB.print(F(" 2nd Air Pollutants Sensor Concentration Estimated: "));
    USB.print(polutants2_particles);
    USB.println(F(" ppm"));

    /* Read and print luminosity from luxes sensor */
    luxes = Gases.getLuxes(INDOOR); /* Sensor location (here INDOOR) */
    USB.print(F("Luminosity: "));
    USB.print(luxes);
    USB.print(F(" luxes "));
    USB.println();

    /* Get and Print the Battery Percentage Level */
    battery_level = PWR.getBatteryLevel();
    USB.print(F("Battery Level: "));
    USB.print(battery_level);
    USB.print(F(" %"));
  
    /* Decoding values to base64 for send preparation */
    polutants_send = (int)(polutants_particles * 1000);
    polutants2_send = (int)(polutants2_particles * 1000);
    co_send = (int)(co_particles * 1000);
    no2_send = (int)(no2_particles * 1000);
    luxes_send = (int)(luxes);
    battery_level = (int)(battery_level);
  
    /* Preparing the bytes vector to send.
     * Store 2 identification vector posistions for each variable*/
    bytes[0] = 0x01;
    bytes[1] = 0x03;

    /* 16 positions of binary right shift @ Memory address 255
     * (0xFF in Hexadecimal) */
    bytes[2] = (polutants_send >> 16) & 0xFF;

    /* 8 positions of binary right shift @ Memory address 255
     *(0xFF in Hexadecimal) */
    bytes[3] = (polutants_send >> 8) & 0xFF;

    /* Store the variable unchanged */
    bytes[4] = polutants_send & 0xFF;

    bytes[5] = 0x02;
    bytes[6] = 0x03;
    bytes[7] = (polutants2_send >> 16) & 0xFF;
    bytes[8] = (polutants2_send >> 8) & 0xFF;
    bytes[9] = polutants2_send & 0xFF;

    bytes[10] = 0x03;
    bytes[11] = 0x03;
    bytes[12] = (co_send >> 16) & 0xFF;
    bytes[13] = (co_send >> 8) & 0xFF;
    bytes[14] = co_send & 0xFF;

    bytes[15] = 0x04;
    bytes[16] = 0x03;
    bytes[17] = (no2_send >> 16) & 0xFF;
    bytes[18] = (no2_send >> 8) & 0xFF;
    bytes[19] = no2_send & 0xFF;

    bytes[20] = 0x05;
    bytes[21] = 0x02;
    bytes[22] = (luxes_send >> 8) & 0xFF;
    bytes[23] = luxes_send & 0xFF;

    bytes[24] = 0x06;
    bytes[25] = 0x01;
    bytes[26] = battery_level & 0xFF;

    USB.println(strlen(bytes));
    USB.println();

    /* Print the "bytes" vector with our data in binary format */
    for (i = 0; i < 28; i++)
        USB.println(bytes[i], BIN);
  
    lora_send(device_eui, app_eui, app_key, size_of_buffer,
    port, socket, (uint8_t *) bytes);

    /* Wait 30000 ms for each data for their acks (2 * RTT) */
    for (i = 0; i < (2 * MINUTESTOSEND); i++) {
        delay(30000);
    }
}