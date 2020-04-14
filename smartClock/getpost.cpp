#include "Arduino.h"
#include "getpost.h"


GetPost::GetPost() {

}

int GetPost::getFan() {
  char request_buffer[IN_BUFFER_SIZE]; //char array buffer to hold HTTP request
  char response_buffer[OUT_BUFFER_SIZE]; //char array buffer to hold HTTP response
  
  sprintf(request_buffer, "GET /"); // YOUR URL HERE
  strcat(request_buffer, ""); // YOUR HOST HERE
  strcat(request_buffer, "\r\n");

  do_http_GET("", request_buffer, response_buffer, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, true); // YOUR HOST HERE


  int ans = atoi(response_buffer);
  return ans;
}


boolean GetPost::post(float light, float temp, float sound, int awakeState)
{
  char request_buffer[IN_BUFFER_SIZE]; //char array buffer to hold HTTP request
  char response_buffer[OUT_BUFFER_SIZE]; //char array buffer to hold HTTP response
  char body[200]; //for body;
  sprintf(body, "light=%f&temp=%f&sound=%f&active=%d", light, temp, sound, awakeState); //generate body, posting to User, 1 step
  int body_len = strlen(body); //calculate body length (for header reporting)
  sprintf(request_buffer, "POST / HTTP/1.1\r\n"); // YOUR URL HERE
  strcat(request_buffer, ""); // YOUR HOST HERE
  strcat(request_buffer, "Content-Type: application/x-www-form-urlencoded\r\n");
  sprintf(request_buffer + strlen(request_buffer), "Content-Length: %d\r\n", body_len); //append string formatted to end of request buffer
  strcat(request_buffer, "\r\n"); //new line from header to body
  strcat(request_buffer, body); //body
  strcat(request_buffer, "\r\n"); //header
  Serial.println(request_buffer);
  do_http_request("", request_buffer, response_buffer, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, true); // YOUR HOST HERE
  Serial.println("AHH");
  Serial.print(response_buffer);
  Serial.println("| AHH");
  Serial.println(strcmp(response_buffer, "keep sleeping\n"));
  return !(strcmp(response_buffer, "keep sleeping\n") == 0);
}

void GetPost::do_http_request(char* host, char* request, char* response, uint16_t response_size, uint16_t response_timeout, uint8_t serial){
  WiFiClient client; //instantiate a client object
  if (client.connect(host, 80)) { //try to connect to host on port 80
    //if (serial) Serial.print(request);//Can do one-line if statements in C without curly braces
    client.print(request);
    memset(response, 0, response_size); //Null out (0 is the value of the null terminator '\0') entire buffer
    uint32_t count = millis();
    while (client.connected()) { //while we remain connected read out data coming back
      client.readBytesUntil('\n',response,response_size);
      //if (serial) Serial.println(response);
      if (strcmp(response,"\r")==0) { //found a blank line!
        break;
      }
      memset(response, 0, response_size);
      if (millis()-count>response_timeout) break;
    }
    memset(response, 0, response_size);  
    count = millis();
    while (client.available()) { //read out remaining text (body of response)
      char_append(response,client.read(),OUT_BUFFER_SIZE);
    }
    //if (serial) Serial.println(response);
    client.stop();
    //if (serial) Serial.println("-----------");  
  }else{
    if (serial) Serial.println("connection failed :/");
    if (serial) Serial.println("wait 0.5 sec...");
    client.stop();
  }
}        




void GetPost::do_http_GET(char* host, char* request, char* response, uint16_t response_size, uint16_t response_timeout, uint8_t serial) {
  WiFiClient client; //instantiate a client object
  if (client.connect(host, 80)) { //try to connect to host on port 80
    //if (serial) Serial.print(request);//Can do one-line if statements in C without curly braces
    client.print(request);
    memset(response, 0, response_size); //Null out (0 is the value of the null terminator '\0') entire buffer
    uint32_t count = millis();
    while (client.connected()) { //while we remain connected read out data coming back
      client.readBytesUntil('\n', response, response_size);
      //if (serial) Serial.println(response);
      if (strcmp(response, "\r") == 0) { //found a blank line!
        break;
      }
      memset(response, 0, response_size);
      if (millis() - count > response_timeout) break;
    }
    memset(response, 0, response_size);
    count = millis();
    while (client.available()) { //read out remaining text (body of response)
      char_append(response, client.read(), OUT_BUFFER_SIZE);
    }
    //if (serial) //Serial.println(response);
    client.stop();
    //if (serial) //Serial.println("-----------");
  } else {
    if (serial) //Serial.println("connection failed :/");
    if (serial) //Serial.println("wait 0.5 sec...");
    client.stop();
  }
}

uint8_t GetPost::char_append(char* buff, char c, uint16_t buff_size) {
  int len = strlen(buff);
  if (len > buff_size) return false;
  buff[len] = c;
  buff[len + 1] = '\0';
  return true;
}
