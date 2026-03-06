#include <Arduino.h>
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

    _client.onMessage([this](char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
        onMessage(topic, payload, properties, len, index, total);
    });
    _client.onConnect([this](bool sessionPresent) {
        LOGI("mqtt", "connected");

        for (auto consumer : _consumers) {
            _client.subscribe(consumer->getTopicName(), 1);
        }
    });
    _client.onDisconnect([this](AsyncMqttClientDisconnectReason reason) {
        LOGI("mqtt", "disconnected");
    });
    _client.setServer(_config.host, _config.port);

    _isConfigured = true;
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

    if (_client.publish(topic, 1, retained, payload)) {
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

    if (_isConfigured && _client.connected()) {
        _client.subscribe(consumer->getTopicName(), 1);
    }
}
