// core/EventBus.cpp
#include "./EventBus.h"

namespace CloudMouse {

void EventBus::initialize() {
    if (initialized) return;
    
    // Create queues
    uiToMainQueue = xQueueCreate(QUEUE_SIZE, sizeof(Event));
    mainToUIQueue = xQueueCreate(QUEUE_SIZE, sizeof(Event));
    
    if (uiToMainQueue && mainToUIQueue) {
        initialized = true;
        Serial.println("✅ EventBus initialized");
    } else {
        Serial.println("❌ EventBus initialization failed!");
    }
}

bool EventBus::sendToUI(const Event& event, TickType_t timeout) {
    if (!initialized) return false;
    return xQueueSend(mainToUIQueue, &event, timeout) == pdPASS;
}

bool EventBus::sendToMain(const Event& event, TickType_t timeout) {
    if (!initialized) return false;
    return xQueueSend(uiToMainQueue, &event, timeout) == pdPASS;
}

bool EventBus::receiveFromUI(Event& event, TickType_t timeout) {
    if (!initialized) return false;
    return xQueueReceive(uiToMainQueue, &event, timeout) == pdPASS;
}

bool EventBus::receiveFromMain(Event& event, TickType_t timeout) {
    if (!initialized) return false;
    return xQueueReceive(mainToUIQueue, &event, timeout) == pdPASS;
}

uint32_t EventBus::getUIQueueCount() const {
    return initialized ? uxQueueMessagesWaiting(mainToUIQueue) : 0;
}

uint32_t EventBus::getMainQueueCount() const {
    return initialized ? uxQueueMessagesWaiting(uiToMainQueue) : 0;
}

void EventBus::logStatus() const {
    Serial.printf("[EventBus] UI Queue: %d, Main Queue: %d\n", 
                  getUIQueueCount(), getMainQueueCount());
}

} // namespace CloudMouse
