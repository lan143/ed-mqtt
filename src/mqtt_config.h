#pragma once

#include <Arduino.h>

#define MQTT_DEFAULT_PORT 1883

#define HOST_LEN 64
#define MQTT_LOGIN_LEN 32
#define MQTT_PASSWORD_LEN 32
#define MQTT_TOPIC_LEN 64

namespace EDMQTT
{
    struct Config
    {
        char host[HOST_LEN] = {0};
        uint16_t port = MQTT_DEFAULT_PORT;
        char login[MQTT_LOGIN_LEN] = {0};
        char password[MQTT_PASSWORD_LEN] = {0};
    };
}
