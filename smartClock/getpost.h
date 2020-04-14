#ifndef getpost_h
#define getpost_h
#include "Arduino.h"
#include <WiFiClientSecure.h>

class GetPost
{
  private:
  const uint16_t IN_BUFFER_SIZE = 1000; //size of buffer to hold HTTP request
  const uint16_t OUT_BUFFER_SIZE = 1000; //size of buffer to hold HTTP response

  const int RESPONSE_TIMEOUT = 6000; //ms to wait for response from host

  public:
    GetPost();
  int getFan();
    boolean post(float light, float temp, float sound, int awakeState);
    void do_http_request(char* host, char* request, char* response, uint16_t response_size, uint16_t response_timeout, uint8_t serial);
    void do_http_GET(char* host, char* request, char* response, uint16_t response_size, uint16_t response_timeout, uint8_t serial);
    uint8_t char_append(char* buff, char c, uint16_t buff_size);
};
#endif
