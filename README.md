# ESPControl
ESPControl — библиотека для Wi-Fi, REST API и интерфейса

Возможности:
- Подключение к Wi-Fi с автопереподключением и fallback в точку доступа.
- REST API с JSON для получения данных сенсоров и вызова действий.
- SPIFFS-интерфейс (веб-панель).
- HTTP-авторизация.
- Сохранение настроек Wi-Fi в энергонезависимую память (Preferences).

## 🔧 Установка
Скачайте и распакуйте архив:
- ESPControl_Full.zip
- Поместите папку ESPControl в директорию: `Documents/Arduino/libraries/`
- Загрузите index.html в SPIFFS (через ESP32 Sketch Data Upload).

📦 Файлы SPIFFS
Обязательно загрузите index.html в SPIFFS перед прошивкой.

🧰 Требуемые библиотеки
- WiFi.h
- WebServer.h
- SPIFFS.h
- Preferences.h
- ArduinoJson.h

- Можно установить их через Library Manager или вручную.

## 🧩 Подключение библиотеки
```
#include <ESPControl.h>
ESPControl esp;
```
### С сохранёнными настройками (или fallback в точку доступа):
```
esp.begin();
```
### С заданными параметрами:
```
esp.begin("yourSSID", "yourPassword");
```
### 🔐 Авторизация
```
esp.setAuth("admin", "admin123");
```
### 🧪 Добавление сенсоров
```
esp.registerSensor("temperature", []() {
  return String(analogRead(34) * 0.1);
});
```
### ⚙️ Добавление действий
```
esp.registerAction("reboot", []() {
  ESP.restart();
  return "OK";
});
```
Вызывается через POST `/api/action` с JSON
```
{"name": "reboot"}
```

### 🌐 Веб-интерфейс
- Файл index.html отображается при обращении к корню /.
- Он использует JS для работы с API.

> [!WARNING]
>## 📡 API Обзор
>`GET /api/sensors`
>Возвращает JSON с текущими значениями всех зарегистрированных сенсоров.
>
>`POST /api/action`
>Принимает JSON {"name": "action_name"}. Выполняет зарегистрированное действие и возвращает результат.

# 🧠 Пример полного скетча
```
#include <ESPControl.h>
ESPControl esp;
void setup() {
  Serial.begin(115200);
  esp.setAuth("admin", "1234");
  esp.begin("MyWiFi", "password");
  esp.registerSensor("uptime", []() {
    return String(millis() / 1000) + " sec";
  });
  esp.registerAction("reboot", []() {
    ESP.restart();
    return "rebooting...";
  });
}
void loop() {
  esp.handle();
}
```

> [!IMPORTANT]
> ❗ Примечания<br/>
> В режиме точки доступа создаётся сеть ESPControl с паролем admin123.<br/>
> Если не удаётся подключиться к Wi-Fi в течение 10 секунд — активируется AP-режим.<br/>
> Все данные сохраняются между перезагрузками.<br/>
