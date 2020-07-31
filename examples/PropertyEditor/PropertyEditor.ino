/*
 * Copyright (c) 2019 Helmut Tschemernjak
 * 30826 Garbsen (Hannover) Germany
 * Licensed under the Apache License, Version 2.0);
 */

#include "NVPropertyProviderInterface.h"
#include "NVProperty.h"


static int GetCommandParms(int *id, char *tbuf, int tbufSize);
static int GetCommandId(int *id);
static const char *getNiceName(int id, int val);



class UserProperty : public NVProperty {
public:
  enum Properties {
    MY_DEVNAME = PRIVATE_RANGE_START,
    MY_CITY,
    MY_APP_PASSWORD,
  };
};

#define ARRAYLEN(arr) (sizeof(arr) / sizeof(0[arr]))

struct propArray {
  int id;
  NVProperty::NVPType type;
  const char *name;
  int valueInt;
  const char *valueStr;
} propArray[] = {
  { NVProperty::RTC_AGING_CAL,      NVProperty::T_32BIT,  "RTC_AGING_CAL", 0, NULL },
  { NVProperty::ADC_VREF,           NVProperty::T_32BIT,  "ADC_VREF", 0, NULL },
  { NVProperty::HARDWARE_REV,       NVProperty::T_32BIT,  "HARDWARE_REV", 0, NULL },
  
  { NVProperty::LORA_DEVICE_ID,     NVProperty::T_32BIT,  "LORA_DEVICE_ID", 0, NULL },  
  { NVProperty::LORA_CODE_ID,       NVProperty::T_32BIT,  "LORA_CODE_ID", 0, NULL },  
  { NVProperty::LORA_REMOTE_ID,     NVProperty::T_32BIT,  "LORA_REMOTE_ID", 0, NULL },
  { NVProperty::LORA_REMOTE_ID_ALT, NVProperty::T_32BIT,  "LORA_REMOTE_ID_ALT", 0, NULL },
  { NVProperty::LORA_RADIO_TYPE,    NVProperty::T_32BIT,  "LORA_RADIO_TYPE", 0, NULL },
  { NVProperty::LORA_FREQUENCY,     NVProperty::T_32BIT,  "LORA_FREQUENCY", 0, NULL },
  { NVProperty::LORA_BANDWIDTH,     NVProperty::T_32BIT,  "LORA_BANDWIDTH", 0, NULL },
  { NVProperty::LORA_SPREADING_FACTOR, NVProperty::T_32BIT,  "LORA_SPREADING_FACTOR", 0, NULL },
  { NVProperty::LORA_TXPOWER,       NVProperty::T_32BIT,  "LORA_TXPOWER", 0, NULL },
  { NVProperty::LORA_FREQUENCY_OFFSET, NVProperty::T_32BIT,  "LORA_FREQUENCY_OFFSET", 0, NULL },
  { NVProperty::LORA_APP_PWD,       NVProperty::T_STR,  "LORA_APP_PWD", 0, NULL },
  { NVProperty::LORA_APP_PWD_ALT,   NVProperty::T_STR,  "LORA_APP_PWD_ALT", 0, NULL },
  
  { NVProperty::LOC_LONGITUDE,      NVProperty::T_STR,  "LOC_LONGITUDE", 0, NULL },
  { NVProperty::LOC_LATITUDE,       NVProperty::T_STR,  "LOC_LATITUDE", 0, NULL },
  { NVProperty::LOC_NAME,           NVProperty::T_STR,  "LOC_NAME", 0, NULL },
  { NVProperty::HOSTNAME,           NVProperty::T_STR,  "HOSTNAME", 0, NULL },

  { NVProperty::WIFI_SSID,          NVProperty::T_STR,    "WIFI_SSID", 0, NULL },
  { NVProperty::WIFI_PASSWORD,      NVProperty::T_STR,    "WIFI_PASSWORD", 0, NULL },
  { NVProperty::WIFI_SSID_ALT,      NVProperty::T_STR,    "WIFI_SSID_ALT", 0, NULL },
  { NVProperty::WIFI_PASSWORD_ALT,  NVProperty::T_STR,    "WIFI_PASSWORD_ALT", 0, NULL },
  { NVProperty::USE_DHCP,           NVProperty::T_32BIT,  "USE_DHCP", 0, NULL },
  { NVProperty::MAC_ADDR,           NVProperty::T_STR,    "MAC_ADDR", 0, NULL },
  { NVProperty::NET_IP_ADDR,        NVProperty::T_STR,    "NET_IP_ADDR", 0, NULL },
  { NVProperty::NET_IP_ROUTER,      NVProperty::T_STR,    "NET_IP_ROUTER", 0, NULL },
  { NVProperty::NET_IP_DNS_SERVER,  NVProperty::T_STR,    "NET_IP_DNS_SERVER", 0, NULL },
  { NVProperty::NET_IP_DNS_SERVER_ALT,  NVProperty::T_STR,    "NET_IP_DNS_SERVER_ALT", 0, NULL },

  { NVProperty::NET_NTP_SERVER,  NVProperty::T_STR,    "NET_NTP_SERVER", 0, NULL },
  { NVProperty::NET_NTP_SERVER_ALT,NVProperty::T_STR,    "NET_NTP_SERVER_ALT", 0, NULL },
  { NVProperty::NET_NTP_GMTOFFSET, NVProperty::T_32BIT,  "NET_NTP_GMTOFFSET", 0, NULL },
  { NVProperty::NET_NTP_DAYLIGHTOFFSET, NVProperty::T_32BIT,  "NET_NTP_DAYLIGHTOFFSET", 0, NULL },

  { NVProperty::MQTT_SERVER,        NVProperty::T_STR,    "MQTT_SERVER", 0, NULL },
  { NVProperty::MQTT_SERVER_ALT,    NVProperty::T_STR,    "MQTT_SERVER_ALT", 0, NULL },
  { NVProperty::MQTT_TOPIC_INFO,    NVProperty::T_STR,    "MQTT_TOPIC_INFO", 0, NULL },
  { NVProperty::MQTT_TOPIC_ALARM,   NVProperty::T_STR,    "MQTT_TOPIC_ALARM", 0, NULL },
  { NVProperty::MQTT_TOPIC_CONTROL, NVProperty::T_STR,    "MQTT_TOPIC_CONTROL", 0, NULL },
  { NVProperty::MQTT_TOPIC_GATEWAY, NVProperty::T_STR,    "MQTT_TOPIC_GATEWAY", 0, NULL },
  { NVProperty::MQTT_TOPIC_5,       NVProperty::T_STR,    "MQTT_TOPIC_5", 0, NULL },
  
  { NVProperty::ALARM_STATUS,       NVProperty::T_32BIT,  "ALARM_STATUS", 0, NULL },

  { NVProperty::VOIP_SERVER,       NVProperty::T_STR,     "VOIP_SERVER", 0, NULL },
  { NVProperty::VOIP_USER,         NVProperty::T_STR,     "VOIP_USER", 0, NULL },
  { NVProperty::VOIP_PASSWORD,     NVProperty::T_STR,     "VOIP_PASSWORD", 0, NULL },
  { NVProperty::VOIP_DEVNAME,      NVProperty::T_STR,     "VOIP_DEVNAME", 0, NULL },
  { NVProperty::VOIP_DIALNO,       NVProperty::T_STR,     "VOIP_DIALNO", 0, NULL },
  { NVProperty::VOIP_DIALNO_ALT,   NVProperty::T_STR,     "VOIP_DIALNO_ALT", 0, NULL },

  { NVProperty::PROG_CMDLINE,      NVProperty::T_STR,     "PROG_CMDLINE", 0, NULL },

  /*
   * A user defined property
   */
  { UserProperty::MY_CITY,          NVProperty::T_STR,    "MY_CITY", 0, NULL },
};
#define ARRAY_SIZE

#if defined(ARDUINO_SAMD_ZERO) || defined(ARDUINO_ARCH_SAMD)
#define MYSERIAL SerialUSB
#else
#define MYSERIAL Serial
#endif

void setup() {
  MYSERIAL.begin(115200);
  while (!MYSERIAL)
    ; // wait for serial port to connect. Needed for native USB port only
 
  UserProperty p;
  
  MYSERIAL.println("\nWelcome to the Property Editor which allows reading/writing/erasing non volatile settings\n");
  MYSERIAL.println("Properties cmds are:  l (list properties), s (set e.g. s20=value), d (delete e.g. d20), q (quit)\n");
  bool done = false;
  while(!done) {
    if (MYSERIAL.available() > 0) {
      int c = MYSERIAL.read();

      switch(c) {
        case 'l': {
          int cnt = ARRAYLEN(propArray);
          MYSERIAL.println("List of Properties:");
          for (int i = 0; i < cnt; i++) {
            char tbuf[128];
            const char *value = "(not set)";
            memset(tbuf, 0, sizeof(tbuf));
            if (propArray[i].type <= NVProperty::T_32BIT) {
                int val = p.GetProperty(propArray[i].id, NVProperty::NVP_ENOENT);
                if (val != NVProperty::NVP_ENOENT) {
                  value = tbuf;
                  snprintf(tbuf, sizeof(tbuf), "%d (0x%x) %s", val, val, getNiceName(propArray[i].id, val));
                }
            } else if (propArray[i].type == NVProperty::T_STR) {
                const char *s = p.GetProperty(propArray[i].id, (const char *)NULL);
                if (s)
                  value = s;
            }
            char tmpbuf[64];
            snprintf(tmpbuf, sizeof(tmpbuf), "%24s %d=",  propArray[i].name, propArray[i].id);
            MYSERIAL.print(tmpbuf);
            MYSERIAL.println(value);
          }
          MYSERIAL.println("");
        }
        break;
        case 'd': {
            int id = 0;
            if (GetCommandId(&id) == -1) {
              MYSERIAL.println("invalid parameter");
              break;    
            }
            int cnt = ARRAYLEN(propArray);
            int slot = -1;
            for (int i = 0; i < cnt; i++) {
              if (propArray[i].id == id) {
                slot = i;
                break;
              }
            }          
            if (slot == -1)
              MYSERIAL.println("Property: " + String(id) + "not found in table");
            else {
              MYSERIAL.println("Deleted Property: " + String(id) + " " + propArray[slot].name);
              p.OpenPropertyStore(true); // enable for write
              p.EraseProperty(id);     
            }
          }
          break;  
        case 's': {
            int id = 0;
            char tbuf[128];
            memset(tbuf, 0, sizeof(tbuf));
            if (GetCommandParms(&id, tbuf, sizeof(tbuf)) == -1) {
              Serial.println("invalid parameter");
              break;
            }
            int cnt = ARRAYLEN(propArray);
            int slot = -1;
            for (int i = 0; i < cnt; i++) {
              if (propArray[i].id == id) {
                slot = i;
                break;
              }
            }
            if (slot == -1)
              MYSERIAL.println("Property: " + String(id) + " not found in table");
            else {
              MYSERIAL.println("Set Property: " + String(propArray[slot].name) + " " + String(id) + " " + String(tbuf));
              p.OpenPropertyStore(true); // enable for write
              if (propArray[slot].type == NVProperty::T_STR) {
                p.SetProperty(id, p.T_STR, tbuf, p.S_FLASH);
              } else if (propArray[slot].type <= NVProperty::T_32BIT) {
                p.SetProperty(id, p.T_32BIT, (int)strtoll(tbuf, NULL, 0), p.S_FLASH); 
              }
            }
          }
          break;
        case 'q':
          done = true;
          break;
        default: 
          continue;
      }
    }
  }
  MYSERIAL.println("\nProperties completed");
}

void loop() {
}


static int GetCommandParms(int *id, char *tbuf, int tbufSize)
{
    bool haveID = false;
    char tmpID[4];
    int cnt = 0;

    memset(tmpID, 0, sizeof(tmpID));

    delay(10); // give the UART some time to receive data
    while (MYSERIAL.available() > 0)
    {
        delay(10); // give the UART some time to receive data
        char c = MYSERIAL.read();

        if (cnt < sizeof(tmpID)-1 && haveID == false && c >= '0' && c <= '9') {
            tmpID[cnt++] = c;
            continue;
        }

        if (c == '=' && cnt <= sizeof(tmpID)-1 && haveID == false) {
          tmpID[cnt]  = 0;
          cnt = 0; // start counting for the string param
          *id = strtol(tmpID, NULL, 0);
          haveID = true;
          continue;
        }

        if (haveID == false)
          goto error;
        
        if (c == '\n' || c == '\r')
          break;
        
        if (cnt >= tbufSize-1) {
          goto error;
        }
        *tbuf++ = c;
        cnt++;
    }
    
    if (cnt > 0 && haveID) { // no more data
      *tbuf = 0;
      return cnt;
    }
error:
  while(MYSERIAL.available() > 0)
    MYSERIAL.read(); // flush input
  return -1;
}


static int GetCommandId(int *id)
{
    char tmpID[4];
    int cnt = 0;

    memset(tmpID, 0, sizeof(tmpID));

    delay(10); // give the UART some time to receive data
    while (MYSERIAL.available() > 0)
    {
        delay(10); // give the UART some time to receive data
        char c = MYSERIAL.read();


        if (cnt < sizeof(tmpID)-1 && c >= '0' && c <= '9') {
            tmpID[cnt++] = c;
            continue;
        }

        if ((c == '\n' || c == '\r') && cnt <= sizeof(tmpID)-1)
          break;

        goto error;
    }
    
    if (cnt > 0 && cnt <=  sizeof(tmpID)-1) { // no more data
        tmpID[cnt]  = 0;
        *id = strtol(tmpID, NULL, 0);
        return *id;
    }
error:
    while(MYSERIAL.available() > 0)
        MYSERIAL.read(); // flush input
    return -1;
}


static const char *getNiceName(int id, int val)
{
  const char *name = "";
  
  switch(id) {
    case NVProperty::LORA_RADIO_TYPE:
      if (val == 1)
        return "RS_Node_Offline";
      else if (val == 2)
        return "RS_Node_Checking";
      else if (val == 3)
        return "RS_Node_Online";
      else if (val == 4)
        return "RS_Station_Basic";
      else if (val == 5)
        return "RS_Station_Server";
      break;
    default:
      break;
  }
  return name;
}
