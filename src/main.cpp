#include <WiFi.h>
#include <ESPAsyncWebServer.h>
//#include <string>
#define ARRAY_LENGTH(array) (sizeof(array) / sizeof(array[0]))
//using namespace std;         //  名前空間指定

const char* ssid = "F660T-knNs-G";
const char* password =  "3iEdCqbq";

const int BUTTON_NUM = 10;
const int LED_PINS[BUTTON_NUM] = {
  2,
  4,
  5,
  12,
  13,
  14,
  15,
  16,
  17,
  18
};
const int BUTTON_PINS[BUTTON_NUM] = {
  19,
  21,
  22,
  23,
  25,
  26,
  27,
  32,
  33,
  39
};
boolean led_states[BUTTON_NUM] = {
  false,
  false,
  false,
  false,
  false,
  false,
  false,
  false,
  false,
  false,
};
unsigned long button_state_time[BUTTON_NUM] = {
  0L,
  0L,
  0L,
  0L,
  0L,
  0L,
  0L,
  0L,
  0L,
  0L,
};
unsigned long button_state[BUTTON_NUM] = {
  false,
  false,
  false,
  false,
  false,
  false,
  false,
  false,
  false,
  false,
};
int count = 0;

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
 
void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len){
 
  if(type == WS_EVT_CONNECT){
    Serial.println("Websocket client connection received");
  } else if(type == WS_EVT_DISCONNECT){
    Serial.println("Client disconnected");
  } else if (type == WS_EVT_DATA) {
    String action = String((char*)data);
    if (action.equals("reset")) {
      memset(led_states, false, BUTTON_NUM);
    }
  } else {
    Serial.println(type);
    Serial.printf("%s\n", (char*)data);
  }
}

void handleIndex(AsyncWebServerRequest *request) {
  request->send(200, "text/html",  "<body><script src=\"http://jirox.net/main.js\"></script></body>");
}
 
void setup(){
  Serial.begin(115200);
 
  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
 
  Serial.println(WiFi.localIP());
 
 	server.on("/", HTTP_ANY, handleIndex);

  ws.onEvent(onWsEvent);
  server.addHandler(&ws);
 
  server.begin();

  for (int i = 0; i < BUTTON_NUM; i++) {
    pinMode(LED_PINS[i], OUTPUT);
  }
  for (int i = 0; i < BUTTON_NUM; i++) {
    pinMode(BUTTON_PINS[i], INPUT_PULLUP);
  }
}

void loop(){
  const unsigned long now = millis();
  boolean new_button_state[BUTTON_NUM];

  // 最初に新しいボタンの状態を作成する
  for (int i = 0; i < BUTTON_NUM; i++) {
    if (now - button_state_time[i] > 50) {
      int val = digitalRead(BUTTON_PINS[i]);
      new_button_state[i] = val == 0;
      if (new_button_state[i] != button_state[i]) {
        button_state_time[i] = now;
      }
    } else {
      new_button_state[i] = button_state[i];
    }
  }

  // 変更されているボタンを検知して、変わっているものがあればwebsocketで全部のボタンの状態を送信する
  boolean is_changed = false;
  for (int i = 0; i < BUTTON_NUM; i++) {
    if (new_button_state[i] != button_state[i]) {
      is_changed = true;
      break;
    }
  }
  if (is_changed) {
    String json("[");
    for (int i = 0; i < BUTTON_NUM; i++) {
      if (new_button_state[i]) {
        json.concat("1");
      } else {
        json.concat("0");
      }
      if (i != BUTTON_NUM - 1) {
        json.concat(",");
      }
    }
    json.concat("]");
    ws.textAll(json);
    Serial.println(json);
  }

  // 更新
  for (int i = 0; i < BUTTON_NUM; i++) {
    button_state[i] = new_button_state[i];
  }
}