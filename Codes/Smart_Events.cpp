/* Libraries and Definitions */
#include <WaspSensorEvent_v30.h>
#include <WaspLoRaWAN.h>
#include <lora_send.h>
#include <lora_join.h>
#define DRY 0.0 /**< Voltage Values */
#define WET 0.2
#define VERY_WET 1.0

/** Socket Selection for the Sensor Probe */
uint8_t socket = SOCKET0;

/** Device parameters for Back-End registration */
char device_eui[] = "72c465dc2f1bcfa7";
char app_eui[] = "72c465dc2f1bcfa7";
char app_key[] = "b5351d67909803c983b66484d25f9362";

/** Define port to use in Back-End: from 1 to 223 */
uint8_t port = 10;

/** Define data payload to send (maximum is up to data rate) */
uint8_t bytes[12] = {0};

/** Setup the data vector */
char data[] = "0102030405060708090A0B0C0D0E0F";

/** Variables */
int i;
int battery_level = 0;
uint8_t value;
float voltage;
float resistance;
uint8_t pir_value = 0;
uint8_t liquid_level = 0;
uint8_t liquid_flag = 0;
uint8_t size_of_buffer = sizeof(bytes) / sizeof(bytes[0]); /* or = 12*/
uint8_t adr_flag = 1; /**< Adaptive Data Rate indicator for lora_join() */

/*
 * Define object for sensor. Choose board socket. 
 * Waspmote OEM. Possibilities for this sensor:
 *  - SOCKET_1 
 *  - SOCKET_2
 *  - SOCKET_3
 *  - SOCKET_4
 *  - SOCKET_6
 * P&S! Possibilities for this sensor:
 *  - SOCKET_A
 *  - SOCKET_C
 *  - SOCKET_D
 *  - SOCKET_E
 */

/* Classes for the sensors */
liquidPresenceClass liquidPresence(SOCKET_6);
pirSensorClass pir(SOCKET_1);
liquidLevelClass liquidLevel(SOCKET_4);

void setup() 
{
    USB.ON();
    USB.println(F("Start program"));

    /* Turn on the sensor board */
    Events.ON();

    lora_join(device_eui, app_eui, app_key, socket, adr_flag);
    
    /* Enable interruptions from the board */
    Events.attachInt();
}

void loop() 
{
    /* Deep sleep mode */
    USB.println(F("enter deep sleep"));
    USB.print(F("Time:"));
    USB.println(RTC.getTime());
    PWR.deepSleep("00:00:30:00", RTC_OFFSET, RTC_ALM1_MODE1, SENSOR_ON);
    USB.println(F("wake up\n"));
    
    /* Check interruption flag from RTC alarm */
    if (intFlag & RTC_INT) {
        USB.print(F("Time:"));
        USB.println(RTC.getTime());
        USB.println(F("-----------------------------"));
        USB.println(F("RTC INT captured"));
        USB.println(F("-----------------------------"));

        /* Read the sensors' data */
        pir_value = pir.readPirSensor();
        value = liquidPresence.readliquidPresence();
        voltage = liquidPresence.readVoltage();
        liquid_level = liquidLevel.readliquidLevel();

        battery_level = PWR.getBatteryLevel();

        liquid_flag = 0x00;
        /* Check if water is detected */
        if ((voltage >= DRY) && (voltage <= WET)) {  
            USB.println(F("Voltage output: Water not detected"));
            liquid_flag = 0x44;
        } else if ((voltage >= WET) && (voltage <= VERY_WET)) {
            USB.println(F("Voltage output: Water detected!"));    
            liquid_flag = 0x57;
        } else if (voltage >= VERY_WET) {
            USB.println(F("Voltage output: "));
            USB.println(F("A lot of water detected!"));
            liquid_flag = 0x56;
        }
    
    /* Preparing the bytes vector to send.
     * Store 2 identification vector posistions for
     * each variable */
    bytes[0] = 0x09;
    bytes[1] = 0x01;
    
    /* store the variable unchanged @ Memory address 255
     * (0xFF in Hexadecimal) */
    bytes[2] = liquid_flag & 0xFF;

    bytes[3] = 0x10;
    bytes[4] = 0x01;
    bytes[5] = liquid_level & 0xFF;

    bytes[6] = 0x11;
    bytes[7] = 0x01;
    bytes[8] = pir_value & 0xFF;

    bytes[9] = 0x06;
    bytes[10] = 0x01;
    bytes[11] = battery_level & 0xFF;
   
    lora_send(device_eui, app_eui, app_key, size_of_buffer,
    port, socket, (uint8_t *) bytes);

    /* Clear Interruption Flag */
    intFlag &= ~(RTC_INT);
    }
  
    /* Check interruption from Sensor Board */
    if (intFlag & SENS_INT) {
        /* Disable interruptions from the board */
        Events.detachInt();
    
        /* Load the interruption flag */
        Events.loadInt();
    
        /* In case the interruption came from the PIR sensor */
        if (pir.getInt()) {
            USB.println(F("-----------------------------"));
            USB.println(F("Interruption from PIR"));
            USB.println(F("-----------------------------"));
        }    

        /* In case the interruption came from the Liquid Level sensor */
        if (liquidLevel.getInt()) {
            /* Initialize */
            liquid_level = 1;
            USB.println(F("-----------------------------"));
            USB.println(F("Interruption from Liquid Level"));
            USB.println(F("-----------------------------"));
        }

        /* In case the interruption came from the Liquid Presence sensor */
        if (liquidPresence.getInt()) {
            USB.println(F("-----------------------------"));
            USB.println(F("Interruption from Liquid Presence"));
            USB.println(F("-----------------------------"));
        }
    
        /* Read the sensors' data */
        pir_value = pir.readPirSensor();
        value = liquidPresence.readliquidPresence();
        voltage = liquidPresence.readVoltage();
        resistance = liquidPresence.readResistance(voltage);
        liquid_level = liquidLevel.readliquidLevel();

        /* Print an info message if there is water */
        if (liquid_level) {
            USB.println(F("FLOTER ON"));
        } else {
            USB.println(F("FLOTER OFF"));
        }
  
        /* Print the info from the liquid presence sensor (value) */
        if (value == 1) {  
            USB.println(F("Sensor output: Water detected!")); 
        } else {
            USB.println(F("Sensor output: Water not detected"));   
        }

        /* Print the info from the liquid presence sensor (voltage) */
        USB.print(F("Voltage: "));
        USB.printFloat(voltage, 3);
        USB.println(" Volts");

        /* Print the info from the liquid presence sensor (resistance) */
        USB.print(F("Resistance: "));
        USB.printFloat(resistance, 3);
        USB.println(" kOhms");

        /* Set the liquidflag variable to zero */
        liquid_flag = 0x00;

         /* Check if water is detected */
         if ((voltage >= DRY) && (voltage <= WET)) {  
             USB.println(F("Voltage output: Water not detected"));
             liquid_flag = 0x44;
         } else if ((voltage >= WET) && (voltage <= VERY_WET)) {
             USB.println(F("Voltage output: Water detected!"));    
             liquid_flag = 0x57;
         } else if (voltage >= VERY_WET) {
             USB.println(F("Voltage output: "));
             USB.println(F("A lot of water detected!"));
             liquid_flag = 0x56;
         }

         /* Print the info from the PIR sensor (value) */
         if (pir_value == 1) {
             USB.println(F("Sensor output: Presence detected"));
         } else {
             USB.println(F("Sensor output: Presence not detected"));
         }
  
         /* Get and Print the Battery Percentage Level */
         battery_level = PWR.getBatteryLevel();
         USB.print(F("Battery Level: "));
         USB.print(battery_level);
         USB.print(F(" %"));
    
         /* Preparing the bytes vector to send.
          * Store 2 identification vector posistions for each variable.
          */
         bytes[0] = 0x09; 
         bytes[1] = 0x01;

         /* Store the variable unchanged @ Memory address 255
          * (0xFF in Hexadecimal) */
         bytes[2] = liquid_flag & 0xFF;

         bytes[3] = 0x10;
         bytes[4] = 0x01;
         bytes[5] = liquid_level & 0xFF;

         bytes[6] = 0x11;
         bytes[7] = 0x01;
         bytes[8] = pir_value & 0xFF;

         bytes[9] = 0x06;
         bytes[10] = 0x01;
         bytes[11] = battery_level & 0xFF;

         lora_send(device_eui, app_eui, app_key, size_of_buffer,
         port, socket, (uint8_t *) bytes);
    
         /* Clear the Interruption Flag */
         intFlag &= ~(SENS_INT);
    
         /* Enable interruptions from the board */
         Events.attachInt();     
    }
}