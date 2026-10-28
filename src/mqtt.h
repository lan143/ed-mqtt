#pragma once

#include <Arduino.h>
#include <AsyncMqttClient.h>
#include <ready.h>
#include <Utils.h>
#include <list>
#include <log/log.h>

#include "mqtt_config.h"
#include "consumer.h"

namespace EDMQTT
{
    class MQTT : public EDHealthCheck::Ready {
    public:
        void init(Config config);
        bool isConnected() { return _client != nullptr && _client->connected(); }
        bool publish(const char* topic, const char* payload, boolean retained);
        void subscribe(Consumer* consumer);
        void connect()
        {
            _wantConnected = true;
            _lastReconnectAttempt = esp_timer_get_time() - RECONNECT_INTERVAL_US;
            tick();
            LOGD("mqtt", "connect");
        }
        void disconnect()
        {
            _wantConnected = false;
            _client->disconnect();
            LOGD("mqtt", "disconnect");
        }

        EDHealthCheck::ReadyResult ready()
        {
            tick();

            return EDHealthCheck::ReadyResult(isConnected(), !isConnected() ? "mqtt is not connected" : "");
        }

    private:
        void onMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total);
        void tick();
        void resetClient();
        void setupClient(AsyncMqttClient* c);

    private:
        static constexpr int64_t RECONNECT_INTERVAL_US = 5000000;
        static constexpr int64_t STUCK_RESET_US = 15000000;

        AsyncMqttClient* _client = new AsyncMqttClient();

        Config _config;
        bool _isConfigured = false;
        bool _wantConnected = false;
        std::list<Consumer*> _consumers;
        int64_t _lastReconnectAttempt = 0;
        int64_t _lastEventUs = 0;
    };
}
