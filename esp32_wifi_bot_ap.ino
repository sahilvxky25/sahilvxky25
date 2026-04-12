#include <WiFi.h>
#include <WebServer.h>

// ─── Access Point Credentials ───────────────────────────────────────
// Laptop connects to THIS network — no router needed
const char* ap_ssid     = "ESP32-Bot";
const char* ap_password = "robot1234";      // min 8 chars; set "" for open AP

WebServer server(80);

// ─── Motor Pins ─────────────────────────────────────────────────────
const int IN1 = 26, IN2 = 27;
const int IN3 = 14, IN4 = 12;
const int ENA = 25;
const int ENB = 33;

// ─── Speed State ────────────────────────────────────────────────────
int speedValue = 150;
int leftSpeed  = 0;
int rightSpeed = 0;

const int maxSpeed = 255;
const int step     = 100;
char lastCommand   = 'S';

// ─── AP Status Timing ───────────────────────────────────────────────
unsigned long lastPrint = 0;
const unsigned long PRINT_INTERVAL = 5000;

// ─── Movement Functions ─────────────────────────────────────────────
void forward() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
  leftSpeed = rightSpeed = speedValue;
}

void backward() {
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH);
  leftSpeed = rightSpeed = speedValue;
}

void leftTurn() {
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
  leftSpeed  = speedValue / 3;
  rightSpeed = speedValue;
}

void rightTurn() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
  leftSpeed  = speedValue;
  rightSpeed = speedValue / 3;
}

void stopBot() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
  leftSpeed = rightSpeed = 0;
}

// ─── Command Handler ────────────────────────────────────────────────
void handleCommand(char cmd) {
  if (cmd == lastCommand && cmd != 'S') {
    speedValue += step;
    if (speedValue > maxSpeed) speedValue = maxSpeed;
  } else if (cmd != 'S') {
    speedValue = step;
  }

  switch (cmd) {
    case 'F': forward();  break;
    case 'B': backward(); break;
    case 'L': leftTurn(); break;
    case 'R': rightTurn(); break;
    case 'S': stopBot();   break;
  }

  lastCommand = cmd;
  ledcWrite(ENA, leftSpeed);
  ledcWrite(ENB, rightSpeed);
}

// ─── HTTP Handlers ───────────────────────────────────────────────────
void addCORSHeaders() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET, OPTIONS");
}

void handleMove() {
  addCORSHeaders();
  if (!server.hasArg("cmd")) { server.send(400, "text/plain", "Missing cmd"); return; }
  String cmdStr = server.arg("cmd");
  if (cmdStr.length() > 0) handleCommand((char)cmdStr[0]);
  server.send(200, "text/plain", "OK");
}

void handleStatus() {
  addCORSHeaders();
  String json = "{";
  json += "\"speed\":"      + String(speedValue)  + ",";
  json += "\"leftSpeed\":"  + String(leftSpeed)   + ",";
  json += "\"rightSpeed\":" + String(rightSpeed)  + ",";
  json += "\"clients\":"    + String(WiFi.softAPgetStationNum()) + ",";
  json += "\"lastCmd\":\""  + String(lastCommand) + "\"";
  json += "}";
  server.send(200, "application/json", json);
}

// ─── Status Print ────────────────────────────────────────────────────
void printAPInfo() {
  Serial.println("================================");
  Serial.print  ("  AP SSID  : "); Serial.println(ap_ssid);
  Serial.print  ("  AP IP    : "); Serial.println(WiFi.softAPIP());
  Serial.print  ("  Clients  : "); Serial.println(WiFi.softAPgetStationNum());
  Serial.println("================================");
}

// ─── Setup ───────────────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);

  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);

  ledcAttach(ENA, 1000, 8);
  ledcAttach(ENB, 1000, 8);

  // ── Start Access Point ──────────────────────────────────────────
  // ESP32 AP fixed IP is always 192.168.4.1
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ap_ssid, ap_password);

  ////Serial.println("\n[AP] Access Point started!");
 // printAPInfo();
 // Serial.println("  Connect laptop/phone to: " + String(ap_ssid));
  //Serial.println("  Then open: http://192.168.4.1");
  //Serial.println("================================\n");

  server.on("/move",   HTTP_GET, handleMove);
  server.on("/status", HTTP_GET, handleStatus);
  server.begin();
}

// ─── Loop ─────────────────────────────────────────────────────────────
void loop() {
  server.handleClient();

  if (millis() - lastPrint >= PRINT_INTERVAL) {
    lastPrint = millis();
    printAPInfo();
  }
}
