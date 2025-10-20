#ifndef TASK_H
#define TASK_H

#include <Arduino.h>
#include <ArduinoJson.h>

class Task {
public:
    int id;
    String uuid;
    String name;
    bool completed;
    bool started;
    String date;
    String status;
    long timerStartedAt;
    int ord;
    bool quoteTime;
    int timeWorked;
    int timeQuoted;
    String workUnit;
    String project;
    String description;

    Task() : id(0), uuid(""), name(""), completed(false), started(false), date(""), status(""), ord(0), timerStartedAt(0), quoteTime(false), timeWorked(0), timeQuoted(0), workUnit(""), project(""), description("") {}

    void fromJson(const JsonObject& json) {

        id = json["id"].as<int>();
        
        // Salva l'attributo "uuid"
        uuid = json["uuid"].as<String>();

        // Salva l'attributo "name"
        name = json["name"].as<String>();

        // Salva "completed" come bool, default a false se non presente
        completed = json.containsKey("completed") ? json["completed"].as<bool>() : false;

        // Salva "started" come bool, default a false se non presente
        started = json.containsKey("started") ? json["started"].as<bool>() : false;

        // Salva la "date"
        date = json["date"].as<String>();
        date.replace(" 00:00:00", "");  // Rimuove il timestamp

        status = json["status"].as<String>();

        ord = json["ord"].as<int>();
        timerStartedAt = json.containsKey("timer_started_at") ? json["timer_started_at"].as<long>() : 0;
        quoteTime = json["quote_time"].as<bool>();
        timeQuoted = json["time_quoted"].as<int>();
        timeWorked = json["time_worked"].as<int>();
        workUnit = json["work_unit_name"].as<String>();
        project = json["project_name"].as<String>();
        description = json["description"].as<String>();
    }

    String toJson() const {
        DynamicJsonDocument doc(1024); // Aumenta se necessario

        // doc["id"] = id;
        // doc["uuid"] = uuid;
        doc["name"] = name;
        // doc["completed"] = completed;
        // doc["started"] = started;
        // doc["date"] = date;
        // doc["status"] = status;
        // doc["ord"] = ord;
        doc["quote_time"] = quoteTime;
        // doc["time_worked"] = timeWorked;
        doc["time_quoted"] = timeQuoted;
        // doc["description"] = description;

        String jsonString;
        serializeJson(doc, jsonString);
        return jsonString;
    }
};

#endif
