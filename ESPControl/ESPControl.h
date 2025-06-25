
#ifndef ESPCONTROL_H
#define ESPCONTROL_H

#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include <FS.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include <functional>
#include <map>

class ESPControl {
public:
  ESPControl() : server(80) {}

  void begin(const char* ssid = nullptr, const char* password = nullptr) {
    SPIFFS.begin(true);
    prefs.begin("espcontrol", false);

    if (ssid && password) {
      WiFi.begin(ssid, password);
      prefs.putString("ssid", ssid);
      prefs.putString("pass", password);
    } else {
      String savedSsid = prefs.getString("ssid", "");
      String savedPass = prefs.getString("pass", "");
      if (savedSsid != "") {
        WiFi.begin(savedSsid.c_str(), savedPass.c_str());
      } else {
        startAP();
        return;
      }
    }

    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < 10000) {
      delay(500);
    }

    if (WiFi.status() != WL_CONNECTED) startAP();

    setupRoutes();
    server.begin();
  }

  void handle() {
    server.handleClient();
  }

  void registerSensor(const String& name, std::function<String()> callback) {
    sensors[name] = callback;
  }

  void registerAction(const String& name, std::function<String()> callback) {
    actions[name] = callback;
  }

  void setAuth(const String& user, const String& pass) {
    authUser = user;
    authPass = pass;
    useAuth = true;
  }

private:
  WebServer server;
  Preferences prefs;
  std::map<String, std::function<String()>> sensors;
  std::map<String, std::function<String()>> actions;
  String authUser, authPass;
  bool useAuth = false;

  bool checkAuth() {
    if (!useAuth) return true;
    if (!server.authenticate(authUser.c_str(), authPass.c_str())) {
      server.requestAuthentication();
      return false;
    }
    return true;
  }

  void startAP() {
    WiFi.softAP("ESPControl", "admin123");
  }

  void setupRoutes() {
    server.on("/", HTTP_GET, [this]() {
      if (!checkAuth()) return;
      File file = SPIFFS.open("/index.html", "r");
      if (!file) {
        server.send(404, "text/plain", "index.html not found");
        return;
      }
      server.streamFile(file, "text/html");
      file.close();
    });

    server.on("/api/sensors", HTTP_GET, [this]() {
      if (!checkAuth()) return;
      DynamicJsonDocument doc(1024);
      for (auto& s : sensors)
        doc[s.first] = s.second();
      String json;
      serializeJson(doc, json);
      server.send(200, "application/json", json);
    });

    server.on("/api/action", HTTP_POST, [this]() {
      if (!checkAuth()) return;
      DynamicJsonDocument doc(512);
      DeserializationError error = deserializeJson(doc, server.arg("plain"));
      if (error) {
        server.send(400, "application/json", "{"error":"Invalid JSON"}");
        return;
      }
      String name = doc["name"].as<String>();
      DynamicJsonDocument resDoc(512);
      if (actions.count(name)) {
        String result = actions[name]();
        resDoc["result"] = result;
      } else {
        resDoc["error"] = "action not found";
      }
      String json;
      serializeJson(resDoc, json);
      server.send(200, "application/json", json);
    });

    server.serveStatic("/", SPIFFS, "/");
  }
};

#endif
