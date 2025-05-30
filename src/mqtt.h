#pragma once

#include <Arduino.h>
#include <ConfigMgr.h>
#include <AsyncMqttClient.h>
#include <ready.h>
#include <Utils.h>
#include <list>

#include "mqtt_config.h"
#include "consumer.h"

namespace EDMQTT
{
    class MQTT : public EDHealthCheck::Ready {
    public:
        MQTT(Config& config) : _config(config) { }
        void init();
        bool isConnected() { return _client.connected(); }
        bool publish(const char* topic, const char* payload, boolean retained);
        void subscribe(Consumer* consumer);
        void connect() { _client.connect(); }

        EDHealthCheck::ReadyResult ready()
        {
            return EDHealthCheck::ReadyResult(isConnected(), !isConnected() ? "mqtt is not connected" : "");
        }

    private:
        void onMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total);

    private:
        AsyncMqttClient _client;
        
        Config& _config;
        bool _isConfigured = false;
        std::list<Consumer*> _consumers;
    };
}
