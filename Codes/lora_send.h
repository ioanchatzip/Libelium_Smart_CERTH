#ifndef LORA_SEND_H
#define LORA_SEND_H

void lora_send(char device_eui[], char app_eui[], char app_key[],
               uint8_t size_of_buffer, uint8_t port, uint8_t socket,
               uint8_t bytes[]);	

#endif
