#include <WaspLoRaWAN.h>
#include <WaspUSB.h>
#include <WaspUSB.cpp>
#include "Waspmote.h"
#include "lora_send.h"

/** 
 * @brief Data Transmission Function.

 * @return Nothing

 * @param device_eui The EUI of the board.
 * @param app_eui The EUI of the application.
 * @param app_key The key needed for the encryption.
 * @param size_of_buffer The size of our data vector.
 * @param port Specifies the port we are using.
 * @param socket Specifies the socket of the board we are using.
 * @param bytes The data vector to be sent.
 *
 */

void lora_send(char device_eui[], char app_eui[], char app_key[],
               uint8_t size_of_buffer, uint8_t port, uint8_t socket,
               uint8_t bytes[])
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

    error = LoRaWAN.joinABP();
    if (error == 0) {
        USB.println(F("Join network OK"));

        /* Send Unconfirmed packet */
        error = LoRaWAN.sendUnconfirmed(port, bytes, size_of_buffer);

        /* Error messages:
         * '6' : Module hasn't joined a network
         * '5' : Sending error
         * '4' : Error with data length   
         * '2' : Module didn't response
         * '1' : Module communication error
         *  If sent, print the OK message and
         *  check if it has been received. If not
         *  sent or received, print the error in decimal format.
         */
         if (error == 0) {
             USB.println(F("Send Unconfirmed packet OK"));  
             if (LoRaWAN._dataReceived == true) {
                 USB.print(F("   There's data on"));
                 USB.print(F(" port number "));
                 USB.print(LoRaWAN._port, DEC);
                 USB.print(F(".\r\n   Data: "));
                 USB.println(LoRaWAN._data);
             }
         } else {
             USB.print(F("Send Confirmed packet error = "));
             USB.println(error, DEC);
         }
    } else {
        USB.print(F("Join network error = ")); 
        USB.println(error, DEC);
    }

    error = LoRaWAN.OFF(socket);
    if (error == 0) {
        USB.println(F("Switch OFF OK"));     
    } else {
        USB.print(F("Switch OFF error = ")); 
        USB.println(error, DEC);
        USB.println();
    }
}