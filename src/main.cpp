#include "app.h"

#define NUM_LEDS 64
#define DATA_PIN D6

const char* ssid = "C3P";
const char* password = "trespatios";
//const char* ssid = "UNE_6EB4";
//const char* password = "9857312100";
CRGB leds[NUM_LEDS];

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

// función para escuchar el websocket
void onEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len){
  if(type == WS_EVT_CONNECT){
    Serial.printf("ws[%s][%u] connect\n", server->url(), client->id());
    client->printf("Hello Client %u :)", client->id());
    client->ping();
  } else if(type == WS_EVT_DISCONNECT){
    Serial.printf("ws[%s][%u] disconnect: %u\n", server->url(), client->id());
  } else if(type == WS_EVT_ERROR){
    Serial.printf("ws[%s][%u] error(%u): %s\n", server->url(), client->id(), *((uint16_t*)arg), (char*)data);
  } else if(type == WS_EVT_PONG){
    Serial.printf("ws[%s][%u] pong[%u]: %s\n", server->url(), client->id(), len, (len)?(char*)data:"");
  } else if(type == WS_EVT_DATA){
    AwsFrameInfo * info = (AwsFrameInfo*)arg;
    String msg = "";
    if(info->final && info->index == 0 && info->len == len){
      //the whole message is in a single frame and we got all of it's data
      Serial.printf("ws[%s][%u] %s-message[%llu]: ", server->url(), client->id(), (info->opcode == WS_TEXT)?"text":"binary", info->len);

      if(info->opcode == WS_TEXT){
        for(size_t i=0; i < info->len; i++) {
          // EN ESTA PARTE LLEGA EL MENSAJE CUANDO ES UN STRING
          //msg += (char) data[i];
          char caracter = (char)data[i];
          if(caracter != ',') msg += caracter;
        }
        //Serial.println("saco array sin comas");
        //Serial.println(msg);
      } else {
        char buff[3];
        for(size_t i=0; i < info->len; i++) {
          sprintf(buff, "%02x ", (uint8_t) data[i]);
          msg += buff ;
        }
      }

      // Serial.printf("%s\n",msg.c_str());
      // prendo los leds
      for(int i=0; i < NUM_LEDS; i++){
         msg.charAt(i) == '0' ? leds[i] = CRGB::Black : leds[i] = CRGB::Red;
      }

      if(info->opcode == WS_TEXT)
        client->text("I got your text message");
      else
        client->binary("I got your binary message");
    } else {
      //message is comprised of multiple frames or the frame is split into multiple packets
      if(info->index == 0){
        if(info->num == 0)
          Serial.printf("ws[%s][%u] %s-message start\n", server->url(), client->id(), (info->message_opcode == WS_TEXT)?"text":"binary");
        Serial.printf("ws[%s][%u] frame[%u] start[%llu]\n", server->url(), client->id(), info->num, info->len);
      }

      Serial.printf("ws[%s][%u] frame[%u] %s[%llu - %llu]: ", server->url(), client->id(), info->num, (info->message_opcode == WS_TEXT)?"text":"binary", info->index, info->index + len);

      if(info->opcode == WS_TEXT){
        for(size_t i=0; i < info->len; i++) {
          msg += (char) data[i];
        }
      } else {
        char buff[3];
        for(size_t i=0; i < info->len; i++) {
          sprintf(buff, "%02x ", (uint8_t) data[i]);
          msg += buff ;
        }
      }
      Serial.printf("%s\n",msg.c_str());

      if((info->index + len) == info->len){
        Serial.printf("ws[%s][%u] frame[%u] end[%llu]\n", server->url(), client->id(), info->num, info->len);
        if(info->final){
          Serial.printf("ws[%s][%u] %s-message end\n", server->url(), client->id(), (info->message_opcode == WS_TEXT)?"text":"binary");
          if(info->message_opcode == WS_TEXT)
            client->text("I got your text message");
          else
            client->binary("I got your binary message");
        }
      }
    }
  }
}

/*
void save(String name, String data, bool clear) {
  File file = SPIFFS.open(name, clear ? "w" : "a+");
  if (!file) {
    Serial.println("file open failed");
    return;
  }
  file.println(data);
  file.close();
}
*/

/*
void clearData(String name) {
  save(name, "", true);
}
*/

/*
String getData(String name) {
  File file = SPIFFS.open(name, "r");
  String line = "";
  if (!file) Serial.println("file open failed");  // Check for errors
  while (file.available()) {
    // wdt_disable();

    // Read only the very first line
    char c = file.read();
    if (c != '\r') {
      line += String(c);
    } else {
      break;
    }
  }

  yield();
  // wdt_enable(1000);
  Serial.println(line);
  return line;
}
*/

/*
void startWifi() {
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  Serial.println(WiFi.localIP());
}
*/

/*
void startServer() {
  server.on("/alarms", HTTP_GET, [](AsyncWebServerRequest *request) {
    String res = "";
    for (int i=0; i<alarmsIndex+1; i++) {
      for (int j=0; j<6; j++) {
        res.concat(alarms[i][j]);
        if (j < 5) {
          res.concat(",");
        }
      }
      res.concat(";");
    }

    request->send(200, "text/plain", res);
  });

  server.on("/time", HTTP_GET, [](AsyncWebServerRequest *request) {
    int H = request->arg(__Z__).toInt();
    int M = request->arg(1).toInt();
    int S = request->arg(2).toInt();
    int MM = request->arg(3).toInt();
    int DD = request->arg(4).toInt();
    int YY = request->arg(5).toInt();

    setTime(H, M, S, DD, MM, YY);

    request->send(200);
  });

  server.on("/alarm", HTTP_GET, [](AsyncWebServerRequest *request) {
    timeDayOfWeek_t DOW = (timeDayOfWeek_t)request->arg(__Z__).toInt();
    int H = request->arg(1).toInt();
    int M = request->arg(2).toInt();
    int interval = request->arg(3).toInt();

    setAlarm(DOW, H, M, interval);

    request->send(200);
  });

  server.begin();
}
*/

void setup(){
  Serial.begin(115200);

  startAP();
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);

  SPIFFS.begin();
  ws.onEvent(onEvent);
  server.addHandler(&ws);
  server.serveStatic("/", SPIFFS, "/");
  server.begin();

  // inicializo la matrix en 0
    for(int i=0; i < NUM_LEDS; i++){
      leds[i] = CRGB::Black;
    }
}

void loop() {
    FastLED.show();
    FastLED.delay(1000/120);
}
