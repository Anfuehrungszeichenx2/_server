#ifndef BUILD_RETURN_H
#define BUILD_RETURN_H
#include <stdint.h>
void build_return(uint8_t **response, uint16_t *len);
int response_add_body(char *str, uint16_t len, uint16_t response_size, uint8_t **response);
int response_add_body2(uint8_t **response, uint8_t response_len);
uint8_t response_add_header(uint8_t **response, uint8_t content_len);
#endif // BUILD_RETURN_H
