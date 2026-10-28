#include <Arduino.h>
#include <esp_timer.h>
#include <log/log.h>
#include "mqtt.h"

void EDMQTT::MQTT::init(Config config)
{
    LOGI("mqtt", "init start");
    _config = config;

    if (strlen(_config.host) == 0) {
        LOGI("mqtt", "host is empty. stop.");

        return;
    }

    setupClient(_client);

    _lastReconnectAttempt = 0;
    _lastEventUs = esp_timer_get_time();

    _isConfigured = true;
}

void EDMQTT::MQTT::setupClient(AsyncMqttClient* c)
{
    c->onMessage([this](char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
        onMessage(topic, payload, properties, len, index, total);
    });
    c->onConnect([this, c](bool sessionPresent) {
        LOGI("mqtt", "connected");

        _lastEventUs = esp_timer_get_time();

        for (auto consumer : _consumers) {
            c->subscribe(consumer->getTopicName(), 1);
        }
    });
    c->onDisconnect([this](AsyncMqttClientDisconnectReason reason) {
        LOGI("mqtt", "disconnected, reason: %d", reason);

        _lastEventUs = esp_timer_get_time();
    });
    c->setServer(_config.host, _config.port);
}

void EDMQTT::MQTT::resetClient()
{
    LOGI("mqtt", "stuck state suspected, forcing reset");
    _client->disconnect(true);
}

void EDMQTT::MQTT::tick()
{
    if (!_isConfigured || !_wantConnected || _client == nullptr || _client->connected()) {
        return;
    }

    int64_t now = esp_timer_get_time();

    if (now - _lastReconnectAttempt < RECONNECT_INTERVAL_US) {
        return;
    }

    _lastReconnectAttempt = now;
    LOGI("mqtt", "attempt to reconnect");

    if (now - _lastEventUs >= STUCK_RESET_US) {
        resetClient();
    }

    _client->connect();
}

void EDMQTT::MQTT::onMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total)
{
    std::string correctPayload(payload, len);
    ESP_LOGD(
        "mqtt",
        "receive message, topic: %s, payload: %s, len: %d, total: %d, index: %d",
        topic,
        correctPayload.c_str(),
        len,
        total,
        index
    );

    for (auto consumer : _consumers) {
        if (strcmp(consumer->getTopicName(), topic) == 0) {
            if (properties.retain && !consumer->acceptRetained()) {
                LOGD("mqtt", "skip retained message, topic: %s", topic);

                continue;
            }

            consumer->consume(correctPayload);
        }
    }
}

bool EDMQTT::MQTT::publish(const char* topic, const char* payload, boolean retained)
{
    if (!isConnected()) {
        LOGE("mqtt", "can't publish message. mqtt is not connected yet.");
        return false;
    }

    LOGD("mqtt", "publish message, topic: %s, payload: %s", topic, payload);

    if (_client->publish(topic, 1, retained, payload)) {
        LOGD("mqtt", "publish successful");

        return true;
    } else {
        LOGE("mqtt", "publish failed");

        return false;
    }
}

void EDMQTT::MQTT::subscribe(Consumer* consumer)
{
    LOGD("mqtt", "subscribe, topic: %s", consumer->getTopicName());

    _consumers.push_back(consumer);

    if (_isConfigured && _client->connected()) {
        _client->subscribe(consumer->getTopicName(), 1);
    }
}
