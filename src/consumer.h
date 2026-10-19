#pragma once

#include "mqtt_config.h"

namespace EDMQTT
{
    class Consumer
    {
    public:
        void init(const char* topic)
        {
            strcpy(_topic, topic);
        }
        virtual void consume(std::string payload) = 0;

        // Return false to have retained messages (re)publish replays skipped, e.g. command topics.
        virtual bool acceptRetained() const { return true; }

        const char* getTopicName() { return _topic; }

    protected:
        char _topic[256] = {0};
    };
}
