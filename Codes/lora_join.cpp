#include <WaspLoRaWAN.h>
#include "Waspmote.h"
#include "lora_join.h"

/** 
 * @brief LoRaWAN Connection Function.
 *
 * @return Nothing
 *
 * @param device_eui The EUI of the board.
 * @param app_eui The EUI of the application.
 * @param app_key The key needed for the encryption.
 * @param socket Specifies the socket of the board we are using.
 * @param adr_flag Indicates if we need Adaptive Data Rate or not.
 *
 */

void lora_join(char device_eui[], char app_eui[], char app_key[],
               uint8_t socket, uint8_t adr_flag)
{
    uint8_t error;
    error = LoRaWAN.ON(socket);

    /* If there is an error, print it in decimal format */
    if (error == 0) {
        USB.println(F("Switch ON OK"));     
    } else {
        USB.print(F("Switch ON error = ")); 
        USB.println(error, DEC);
    }

    if (adr_flag == 1) {
        /* Enable Adaptive Data Rate (ADR) */
        error = LoRaWAN.setADR("on");
        if(error == 0) {
            USB.print(F("Adaptive Data Rate enabled OK. "));
            USB.print(F("ADR:"));
            USB.println(LoRaWAN._adr, DEC);
        } else {
            USB.print(F("Enable data rate error = ")); 
            USB.println(error, DEC);
        }
    }

    /* Set the Device EUI as input */
    error = LoRaWAN.setDeviceEUI(device_eui);
    if (error == 0) {
        USB.println(F("Device EUI set OK"));     
    } else {
        USB.print(F("Device EUI set error = ")); 
        USB.println(error, DEC);
    }

    /* Set Application EUI as input */
    error = LoRaWAN.setAppEUI(app_eui);
    if (error == 0) {
        USB.println(F("Application EUI set OK"));     
    } else {
        USB.print(F("Application EUI set error = ")); 
        USB.println(error, DEC);
    }

    /* Set Application Session Key as input */
    error = LoRaWAN.setAppKey(app_key);
    if (error == 0) {
        USB.println(F("Application Key set OK"));     
    } else {
        USB.print(F("Application Key set error = ")); 
        USB.println(error, DEC);
    }

    /* Join OTAA to negotiate keys with the server */
    error = LoRaWAN.joinOTAA();
    if (error == 0) {
        USB.println(F("Join network OK"));         
    } else {
        USB.print(F("Join network error = ")); 
        USB.println(error, DEC);
    }

    /* Save configuration to the module's non-volatile memory */
    error = LoRaWAN.saveConfig();
    if (error == 0) {
        USB.println(F("Save configuration OK"));     
    } else {
        USB.print(F("Save configuration error = ")); 
        USB.println(error, DEC);
    }

    error = LoRaWAN.OFF(socket);
    if (error == 0) {
        USB.println(F("Switch OFF OK"));     
    } else {
        USB.print(F("Switch OFF error = ")); 
        USB.println(error, DEC);
    }

    USB.println(F("\n--------------------------------------------------"));
    USB.println(F("Module configured"));
    USB.println(F("After joining through OTAA, the module and the"));
    USB.println(F(" network exchanged the Network Session Key"));
    USB.println(F(" and the Application Session Key which are needed"));
    USB.println(F(" to perform communications. After that, 'ABP mode'"));
    USB.println(F(" is used to join the network and send messages"));
    USB.println(F(" after powering on the module"));
    USB.println(F("\n--------------------------------------------------"));
    USB.println();  
}