// core/EventBus.h
#ifndef EVENT_BUS_H
#define EVENT_BUS_H

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include "Events.h"

namespace CloudMouse {

class EventBus {
public:
    static EventBus& instance() {
        static EventBus bus;
        return bus;
    }
    
    void initialize();
    
    // Inter-task communication
    bool sendToUI(const Event& event, TickType_t timeout = 0);
    bool sendToMain(const Event& event, TickType_t timeout = 0);
    
    bool receiveFromUI(Event& event, TickType_t timeout = 0);
    bool receiveFromMain(Event& event, TickType_t timeout = 0);
    
    // Queue monitoring
    uint32_t getUIQueueCount() const;
    uint32_t getMainQueueCount() const;
    
    void logStatus() const;
    
private:
    QueueHandle_t uiToMainQueue = nullptr;
    QueueHandle_t mainToUIQueue = nullptr;
    
    static const uint32_t QUEUE_SIZE = 10;
    bool initialized = false;
};

} // namespace CloudMouse

#endif
