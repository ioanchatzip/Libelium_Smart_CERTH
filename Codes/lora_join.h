#ifndef LORA_JOIN_H
#define LORA_JOIN_H

void lora_join(char device_eui[], char app_eui[], char app_key[],
               uint8_t socket, uint8_t adr_flag);

#endif
