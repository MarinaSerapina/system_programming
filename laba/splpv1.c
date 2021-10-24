
#include "splpv1.h"
#include <string.h>

char good_data[] = "0000000000000000000000000000000000000000000000101111111111000000000000000000000000000000000000000111111111111111111111111110000";
char good_b64[] = "0000000000000000000000000000000000000000000100011111111111000000011111111111111111111111111000000111111111111111111111111110000";
char good_version[] = "0000000000000000000000000000000000000000000000011111111111000000000000000000000000000000000000000000000000000000000000000000000";
char ver[] = "VERSION ";
char connect[] = "CONNECT";
char connect_ok[] = "CONNECT_OK";
char get_ver[] = "GET_VER";
char get_data[] = "GET_DATA";
char get_file[] = "GET_FILE";
char get_command[] = "GET_COMMAND";
char get_b64[] = "GET_B64";
char disconnect[] = "DISCONNECT";
char disconnect_ok[] = "DISCONNECT_OK";

static  enum  Conditional current_conditional = 1;
char * current_text;

enum Conditional
{
  INIT = 1,
  CONNECTING,
  CONNECTED,
  WAITING_VER,
  WAITING_DATA,
  WAITING_B64_DATA,
  DISCONNECTING
};

inline int validating_version(const char* version)
{
  int i = 8;
  if (version[0] != ver[0] && version[1] != ver[1] && version[2] != ver[2] && version[3] != ver[3] && version[4] != ver[4] && version[5] != ver[5] &&  version[6] != ver[6] && version[7] != ver[7])
    return MESSAGE_INVALID;
  if (version[i] != '\0') {
    while (good_version[version[i]] == '1')
      i++;
    if (version[i] == '\0')
      return MESSAGE_VALID;
  }
  return MESSAGE_INVALID;
}

inline int validating_data(const char* comand, const char* data)
{
  int i = 0, j = 0,len = 0;
  while (comand[i] == data[i])
    i++;
  len = i;
  if (data[i] == ' ')
  {
    i++;
    while (good_data[(int)data[i]] == '1')
      i++;
    if (data[i] == ' ') {
      i++;
      while (comand[j] == data[i] && data[i] != '\0') {
        i++;
        j++;
      }
      if(len == j)
        return MESSAGE_VALID;
    }
  }
  return MESSAGE_INVALID;
}

inline int validating_B64(const char* b64)
{
  int i = 5, flag = 0;
  if (b64[0] != 'B' && b64[1] != '6' && b64[2] != '4' && b64[3] != ':' && b64[4] != ' ')
    return MESSAGE_INVALID;
  if (b64[i] != '\0') {
    while (good_b64[(int)b64[i]] == '1')
      i++;
    if (b64[i] == '\0')
      flag = 1;
    else if (b64[i] == '=') {
      i++;
      if (b64[i] == '\0')
        flag = 1;
      else if (b64[i] == '=') {
        i++;
        if (b64[i] == '\0')
          flag = 1;
      }
    }
    i--;
    if (flag == 1 && ((i & 3) == 0))
      return MESSAGE_VALID;
  }
  return MESSAGE_INVALID;
}


enum test_status validate_message(struct Message *msg)
{
  switch (current_conditional)
  {
    case 1:
    {
      if (msg->direction == A_TO_B && strcmp(msg->text_message, connect) == 0)
      {
        current_conditional = CONNECTING;
        return MESSAGE_VALID;
      }
      break;
    }
    case 2:
    {
      if (msg->direction == B_TO_A && strcmp(msg->text_message, connect_ok) == 0)
      {
        current_conditional = CONNECTED;
        return MESSAGE_VALID;
      }
      break;
    }
    case 3:
    {
      if (msg->direction == A_TO_B)
      {
        if (strcmp(msg->text_message, get_b64) == 0) {
          current_conditional = WAITING_B64_DATA;
          return MESSAGE_VALID;
        }

        if (strcmp(msg->text_message, get_file) == 0 || strcmp(msg->text_message, get_command) == 0 || strcmp(msg->text_message, get_data) == 0) {
          current_conditional = WAITING_DATA;
          current_text = msg->text_message;
          return MESSAGE_VALID;
        }
        if (strcmp(msg->text_message, get_ver) == 0) {
          current_conditional = WAITING_VER;
          return MESSAGE_VALID;
        }
        if (strcmp(msg->text_message, disconnect) == 0) {
          current_conditional = DISCONNECTING;
          return MESSAGE_VALID;
        }
      }
      break;
    }
    case 4:
    {
      if (msg->direction == B_TO_A && validating_version(msg->text_message)) {
        current_conditional = CONNECTED;
        return MESSAGE_VALID;
      }
      break;
    }
    case 5:
    {
      if (msg->direction == B_TO_A && validating_data(current_text, msg->text_message)) {
        current_conditional = CONNECTED;
        return MESSAGE_VALID;
      }
      break;
    }
    case 6:
    {
      if (msg->direction == B_TO_A && validating_B64(msg->text_message)) {
        current_conditional = CONNECTED;
        return MESSAGE_VALID;
      }
      break;
    }
    case 7:
    {
      if (msg->direction == B_TO_A && strcmp(msg->text_message, disconnect_ok) == 0)
      {
        current_conditional = INIT;
        return MESSAGE_VALID;
      }
      break;
    }
  }
  current_conditional = INIT;
  return MESSAGE_INVALID;
}