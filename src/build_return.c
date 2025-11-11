#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

//zwei sterne bedeutet das wir eine addresse uebergeben
int response_add_body(char *str, uint16_t len, uint16_t response_size, uint8_t **response) {
	//hier erstellen wir den pointer der auf unseren buffer zeigt.
	*response = realloc(*response, response_size + len);
	if(*response == NULL)
	{
		fprintf(stderr, "realloc error\n");
		exit(-1);	
	}
	memcpy((*response) + response_size, str, len);

	//fprintf(stderr, "t: %s \n", *response);	
	return len;
	//todo: fehler fangen
}
uint8_t response_add_header(uint8_t **response, uint8_t content_len) {
	//hier besser machen
	*response = malloc(400 * sizeof(uint8_t));
	snprintf(*response,400 * sizeof(uint8_t), "HTTP/1.1 200 MEOW\r\n"
			"Content-Length: %d\r\n"
			"Content-Type: text/html; charset=utf-8\r\n"
			"\r\n", content_len);
	uint8_t header_len = strlen(*response);
	*response = realloc(*response, header_len * sizeof(uint8_t));
	return header_len;
}
int response_add_body2(uint8_t **response, uint8_t response_len) {
//begin stolen from https://stackoverflow.com/questions/174531/how-to-read-the-content-of-a-file-to-a-string-in-c
	char * buffer = 0;
	long length;
	FILE * f = fopen ("index.html", "rb");
	if (f)
	{
	  fseek (f, 0, SEEK_END);
	  length = ftell (f);
	  fseek (f, 0, SEEK_SET);
	  buffer = malloc (length);
	  if (buffer)
	  {
		fread (buffer, 1, length, f);
	  }
	  fclose (f);
	}

	if (buffer)
	{
		memcpy((*response) + response_len, buffer, length);
		free(buffer);
	  // start to process your data / extract strings here...
	}
	return length;
	//end stolen
}

//hier noch input uebergeben lassen
void build_return(uint8_t **response, uint16_t *len) {
//	char body[111] = "Hallo Welt.";
	uint8_t *body;
	int body_len = response_add_body2(&body,0);			
	uint16_t response_size = response_add_header(response, body_len);	
	*len = response_size + response_add_body2(response, response_size);
}
