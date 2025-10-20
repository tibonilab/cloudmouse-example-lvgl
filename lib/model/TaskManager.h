#ifndef TASKMANAGER_H
#define TASKMANAGER_H

#include <Arduino.h>
#include <vector>
#include <algorithm>
#include "Task.h"
#include "../core/EventBus.h"
#include "../core/Events.h"
#include "../helper/JsonHelper.h"
#include "../prefs/PreferencesManager.h"
#include "../helper/NTPManager.h"
#include <ArduinoJson.h>

class TaskManager {
private:
  std::vector<Task> tasks;
  std::vector<Task> defaultTasks;
  unsigned long lastDataUpdate = 0;

  void sortTasks() {
    std::sort(tasks.begin(), tasks.end(), [](const Task& a, const Task& b) {
      return a.ord < b.ord;
    });
  }

public:
  TaskManager() {}

  // 🆕 Processa eventi in arrivo
  // void handleAPIResponse(const CloudMouse::Event& event);

  bool updateFromWebSocket(const String& jsonData);
  bool updateFromHTTP(const String& jsonData, CloudMouse::EventType responseType);


  // Getters
  String getToday() {
    return CloudMouse::NTPManager::getCurrentDate();
  }
  int getTaskCount() const { return tasks.size(); }
  size_t getDefaultTasksCount() const {
    return defaultTasks.size();
  }
  const std::vector<Task>& getTasks() const { return tasks; }
  const Task* getCurrentTask() const;        
  const std::vector<Task>& getDefaultTasks() const {
    return defaultTasks;
  }

  const Task* getTaskByUuid(const String& uuid) const;
  bool hasRunningTask();
  Task* catchRunningTask();

  long getLastDataUpdate() const {
    return lastDataUpdate;
  }

  const int getTaskIndexByUuid(const String& uuid) const;

private:
  // 🆕 Metodi privati che gestiscono i dati e emettono eventi
  void updateTaskList(const String& jsonData);
  void updateDefaultTasks(const String& jsonData);
  void updateUserData(const String& jsonData);
  void updateSingleTask(const String& jsonData);

  bool updateTask(const Task& updatedTask);
  void removeTaskById(int taskId);

  void emitTaskListUpdated();
  void emitTaskDataChanged(const String& taskUuid);

};

#endif