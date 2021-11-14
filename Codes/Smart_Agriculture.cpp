/* Libraries and Definitions */
#include <WaspLoRaWAN.h>
#include <WaspSensorAgr_v30.h>
#include <lora_send.h>
#include <lora_join.h>
#define SENDRATEMINUTES 30 /**< Time in minutes */

/** Socket Selection for the Sensor Probe */
uint8_t socket = SOCKET0;

/** Device parameters for Back-End registration */
char device_eui[] = "72c465dc9a6d5390";
char app_eui[] = "72c465dc9a6d5390";
char app_key[] = "1503835a0767d925cd3c8f9f2bcbb6b5";

/** Define port to use in Back-End: from 1 to 223 */
uint8_t port = 10;

/** Define data payload to send (maximum is up to data rate) */
uint8_t bytes[45] = {0};
char vane_str[10] = {0};

/** Variables */
uint8_t j = 0;
uint8_t i = 0;
int battery_level = 0;
float wetness = 0;
float watermark1;
float temperature = 0;
float anemometer;
float pluviometer1; /**< mm in current hour */
float pluviometer2; /**< mm in previous hour */
float pluviometer3; /**< mm in last 24 hours */
uint8_t cbar = 2;
int vane;
float BME_temperature, humidity, pressure;
int pending_pulses;
uint8_t counter = 0;
uint8_t vane_flag = 0;
uint8_t size_of_buffer = 0; /* it changes from 42 to 44 */
uint8_t adr_flag = 1; /**< Adaptive Data Rate indicator for lora_join() */

/* Classes for the sensors */
leafWetnessClass lwSensor;
weatherStationClass vaneSensor;
weatherStationClass weather;
pt1000Class pt1000Sensor;
watermarkClass wmSensor1(SOCKET_2);

void setup() 
{
    USB.ON();
    USB.println(F("Start program"));
    USB.println(F("------------------------------------"));
    USB.println(F("Module configuration"));
    USB.println(F("------------------------------------\n"));

   /* Turn on the sensor board */
    Agriculture.ON();
    lora_join(device_eui, app_eui, app_key, socket, adr_flag);
    
    get_measurements();
    
    create_bytes();
    
    lora_send(device_eui, app_eui, app_key, size_of_buffer,
    port, socket, bytes);
  
    RTC.ON();
    USB.println(RTC.getTime());
}

void loop() 
{
    /* Sleep Arguments */
    Agriculture.sleepAgr("00:00:00:25", RTC_ABSOLUTE, RTC_ALM1_MODE5,
                         SENSOR_ON, SENS_AGR_PLUVIOMETER);

    /* Check interruption from RTC */
    if (intFlag & RTC_INT) {
        counter++;
        USB.print(F("counter ++"));
        USB.println(RTC.getTime());
        RTC.ON();

        if (counter == SENDRATEMINUTES) {
            USB.println(F("+++ RTC interruption +++"));

            Agriculture.ON();
            USB.println(F("Time FOR Sensoring"));
            USB.println(RTC.getTime()); /* sensoring time */

            get_measurements();

            create_bytes();

            lora_send(device_eui, app_eui, app_key, size_of_buffer,
            port, socket, bytes);
      
            counter = 0;
        }
    /* Clear Interruption Flag */
    intFlag &= ~(RTC_INT); 
    }

    /* Check interruption from Sensor Board */
    if (intFlag & PLV_INT) {
        USB.println(F("+++ PLV interruption +++"));

        pending_pulses = intArray[PLV_POS];

        USB.print(F("Number of pending pulses:"));
        USB.println(pending_pulses);

        for (int k = 0; k < pending_pulses; k++) {
            /* Enter pulse information inside class structure */
            weather.storePulse();

            /* Decrease the Number of Pulses */
            intArray[PLV_POS]--;
        }
    /* Clear Interruption Flag */
    intFlag &= ~(PLV_INT); 
    }
    USB.println(F("Time Elapsed"));
    USB.println(RTC.getTime());
}

/** Function for creating the ready to send bytes vector 
 * @return Nothing
 */
void create_bytes()
{   
    /* Temprature variable for soil */
    int16_t   temperature_send = 0;

    /* Temprature variable for atmosphere */
    int16_t   BME_temperature_send = 0; 
    uint32_t  anemometer_send = 0;
    uint32_t  pluviometer1_send = 0;
    uint16_t  wetness_send = 0;
    uint16_t  humidity_send = 0;
    uint32_t  pressure_send = 0;
    uint8_t   soiltension_send = 0;

    /** Decoding values to base64 for send preparation */
    temperature_send = (int16_t)(temperature * 100);
    BME_temperature_send = (int16_t)(BME_temperature * 100);
    anemometer_send = (uint32_t)(anemometer * 100);
    pluviometer1_send = (uint32_t)(pluviometer1 * 1000) ; 
    wetness_send = (uint32_t)(wetness * 100);
    humidity_send = (uint32_t)(humidity * 100);
    pressure_send = (uint32_t)(pressure);
    soiltension_send = cbar; 
    
    /* Preparing the bytes vector to send */
    /* Store 2 identification vector posistions for each variable */
    bytes[0] = 0x12; 
    bytes[1] = 0x03;

    /* 16 positions of binary right shift @ Memory address 255
     * (0xFF in Hexadecimal) */
    bytes[2] = (pluviometer1_send >> 16) & 0xFF;
    
    /* 8 positions of binary right shift @ Memory address 255
     * (0xFF in Hexadecimal) */
    bytes[3] = (pluviometer1_send >> 8) & 0xFF;
    
    /* store the variable unchanged */
    bytes[4] = pluviometer1_send & 0xFF; 
    
    bytes[5] = 0x13;
    bytes[6] = 0x03;
    bytes[7] = (anemometer_send >> 16) & 0xFF;
    bytes[8] = (anemometer_send >> 8) & 0xFF;
    bytes[9] = anemometer_send & 0xFF;

    bytes[10] = 0x14;
    bytes[11] = 0x03;
    bytes[12] = (pressure_send >> 16) & 0xFF;
    bytes[13] = (pressure_send >> 8) & 0xFF;
    bytes[14] = pressure_send & 0xFF;

    bytes[15] = 0x15;
    bytes[16] = 0x02;
    bytes[17] = (BME_temperature_send >> 8) & 0xFF;
    bytes[18] = BME_temperature_send & 0xFF;

    bytes[19] = 0x16;
    bytes[20] = 0x02;
    bytes[21] = (temperature_send >> 8) & 0xFF;
    bytes[22] = temperature_send & 0xFF;

    bytes[23] = 0x17;
    bytes[24] = 0x03;
    bytes[25] = (wetness_send >> 16) & 0xFF;
    bytes[26] = (wetness_send >> 8) & 0xFF;
    bytes[27] = wetness_send & 0xFF;

    bytes[28] = 0x18;
    bytes[29] = 0x03;
    bytes[30] = (humidity_send >> 16) & 0xFF;
    bytes[31] = (humidity_send >> 8) & 0xFF;
    bytes[32] = humidity_send & 0xFF;

    bytes[33] = 0x19;
    bytes[34] = 0x01;
    bytes[35] = soiltension_send & 0xFF;
    
    bytes[36] = 0x06;
    bytes[37] = 0x01;
    bytes[38] = battery_level & 0xFF;

    i = 0;
    /* Increase i until the vane_str vector ends */
    while (vane_str[i] != 0)
        i++;

    USB.println("print i");
    USB.print(i);
    
    bytes[39] = 0x20;
    bytes[40] = i & 0xFF;

    /* If i = 1, store the first element of the
     * vector @ Memory address 255 (0xFF in Hexadecimal) */
    if (i == 1) {
        bytes[41] = (vane_str [0]) & 0xFF;
        size_of_buffer = 42;
    } else if (i == 2) {
        bytes[41] = (vane_str [0]) & 0xFF;
        bytes[42] = (vane_str [1]) & 0xFF;
        size_of_buffer = 43;
    } else if(i == 3) {
        bytes[41] = (vane_str [0]) & 0xFF;
        bytes[42] = (vane_str [1]) & 0xFF;
        bytes[43] = (vane_str [2]) & 0xFF;
        size_of_buffer = 44;
    }
}

/** Function for getting the measurements from the Sensors 
 * @return Nothing
 */
void get_measurements()
{
    /* Function for Reading the wind sensor values */
    measureSensors();
    
    /* Read the Watermarks sensors one by one */
    USB.println(F("Wait for Watermark 1..."));
    watermark1 = wmSensor1.readWatermark();      

    USB.print(F("Watermark 1 - Frequency: "));
    USB.print(watermark1);
    USB.println(F(" Hz"));

    /* Calculate the cbar */
    if (watermark1 < 300) {
        /* too low values */
        cbar = 200;
    } else if (watermark1 > 7600) {
        /* too high values */
        cbar = 0;
    } else { /* Normal Values */
        /* Calculation Formula */
        cbar = (int)((150940 - (19.74 * watermark1))
         / ((2.8875 * watermark1) - 137.5));
    }
    USB.println(F("Cbar is"));
    USB.println(cbar);

    temperature = pt1000Sensor.readPT1000();  
    USB.print(F("PT1000: "));
    USB.printFloat(temperature, 3);
    USB.println(F(" ºC")); 

    wetness = lwSensor.getLeafWetness();
    USB.print("Leaf Wetness: ");
    USB.println(wetness);
  
    BME_temperature = Agriculture.getTemperature();
    USB.print(F("BMETemperature: "));
    USB.print(BME_temperature);
    USB.println(F(" Celsius"));


    humidity = Agriculture.getHumidity();
    USB.print(F("Humidity: "));
    USB.print(humidity);
    USB.println(F(" %"));

    pressure = Agriculture.getPressure();  
    USB.print(F("Pressure: "));
    USB.print(pressure);
    USB.println(F(" Pa"));  
    USB.println();

    battery_level = PWR.getBatteryLevel();
    USB.print(F("Battery Level: "));
    USB.print(battery_level);
    USB.print(F(" %"));
}

/** Function for Reading the wind sensor values 
 * @return Nothing
 */
void measureSensors()
{  
    USB.println(F("------------- Measurement process ------------------"));

    anemometer = weather.readAnemometer();
    USB.print(F("Anemometer: "));
    USB.print(anemometer);
    USB.println(F("km/h"));
  
    pluviometer1 = weather.readPluviometerCurrent();
    USB.print(F("Current hour accumulated rainfall (mm/h): "));
    USB.println( pluviometer1);

    vaneSensor.getVaneFiltered();

    /* Initialization of vane_str vector to zero */
    for(i = 0; i < 10; i++)
        vane_str[i] = 0;
        
    /* vane_flag depends on the wind direction */
    switch(vaneSensor.vaneDirection) {
    case SENS_AGR_VANE_N:
                        snprintf(vane_str, sizeof(vane_str), "N"); 
                        vane_flag = 1;
                        break;
    case SENS_AGR_VANE_NNE:
                          snprintf(vane_str, sizeof(vane_str), "NNE");
                          vane_flag = 2; 
                          break;  
    case SENS_AGR_VANE_NE:
                         snprintf(vane_str, sizeof(vane_str), "NE");
                         vane_flag = 3;
                         break;    
    case SENS_AGR_VANE_ENE:
                          snprintf(vane_str, sizeof(vane_str), "ENE");
                          vane_flag = 4;
                          break;      
    case SENS_AGR_VANE_E:
                        snprintf(vane_str, sizeof(vane_str), "E");
                        vane_flag = 5;
                        break;    
    case SENS_AGR_VANE_ESE:
                          snprintf(vane_str, sizeof(vane_str), "ESE");
                          vane_flag = 6; 
                          break;  
    case SENS_AGR_VANE_SE:
                         snprintf(vane_str, sizeof(vane_str), "SE");
                         vane_flag = 7;
                         break;    
    case SENS_AGR_VANE_SSE:
                           snprintf(vane_str, sizeof(vane_str), "SSE");
                           vane_flag = 8;
                           break;   
    case SENS_AGR_VANE_S:
                        snprintf(vane_str, sizeof(vane_str), "S");
                        vane_flag = 9;
                        break; 
    case SENS_AGR_VANE_SSW:
                          snprintf(vane_str, sizeof(vane_str), "SSW");
                          vane_flag = 10;
                          break; 
    case SENS_AGR_VANE_SW:
                         snprintf(vane_str, sizeof(vane_str), "SW");
                         vane_flag = 11;
                         break;  
    case SENS_AGR_VANE_WSW:
                          snprintf(vane_str, sizeof(vane_str), "WSW");
                          vane_flag = 12;
                          break; 
    case SENS_AGR_VANE_W:
                        snprintf(vane_str, sizeof(vane_str), "W");
                        vane_flag = 13;
                        break;   
    case SENS_AGR_VANE_WNW:
                          snprintf(vane_str, sizeof(vane_str), "WNW");
                          vane_flag = 14;
                          break; 
    case SENS_AGR_VANE_NW:
                         snprintf(vane_str, sizeof(vane_str), "WN");
                         vane_flag = 15;
                         break;
    case SENS_AGR_VANE_NNW:
                          snprintf(vane_str, sizeof(vane_str), "NNW");
                          vane_flag = 16;
                          break;  
    default:
           snprintf(vane_str, sizeof(vane_str), "ERR"); /* Error Case */
           vane_flag = 0;
           break;
    }
    USB.println(vane_str);
    USB.println(F("--------------------------------------------------\n"));
}