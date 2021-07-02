#include <sstream>
#include <Arduino.h>
#include <WiFi.h>
#include <MQTT.h>
#include <string>

#include "credentials.hpp"

WiFiClient net;
MQTTClient client;

std::string GATE_ID = "0";
std::string cmdStr = "";

int offpin = 22;
int onpin = 23;
uint8_t A_pin = 35;
uint8_t B_pin = 34;
uint8_t C_pin = 39;
uint8_t D_pin = 36;
uint8_t remote_enable_pin = 32;
uint8_t switch_pin = 16;

int lastTimeKeyPressed = 0;
enum class Key {
    A, B, C, D, E, NONE
};
std::string keyCmd;

void IRAM_ATTR on_key(Key k) {
    Serial.println("Getting keypress interrupt");
    int timeNow = millis();
    if (timeNow - lastTimeKeyPressed > 200) {
        switch (k) {
            case Key::A: keyCmd = "A"; break;
            case Key::B: keyCmd = "B"; break;
            case Key::C: keyCmd = "C"; break;
            case Key::D: keyCmd = "D"; break;
            default: keyCmd = "E"; break;
        }
    }
    lastTimeKeyPressed = timeNow;
}

void IRAM_ATTR on_A() {
    on_key(Key::A);
}
void IRAM_ATTR on_B() {
    on_key(Key::B);
}
void IRAM_ATTR on_C() {
    on_key(Key::C);
}
void IRAM_ATTR on_D() {
    on_key(Key::D);
}
void IRAM_ATTR on_key() {
    on_key(Key::E);
}

void publish(const std::string& topic, const std::string& payload) {
    auto fullTopic = topic + "/" + GATE_ID;
    client.publish(fullTopic.c_str(), payload.c_str());
}

void subscribe(const std::string& topic) {
    auto fullTopic = topic + "/" + GATE_ID;
    client.subscribe(fullTopic.c_str());
}

void connect() {
    Serial.print("checking wifi...");
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(1000);
    }
    Serial.println("\nconnected! Local IP: ");
    Serial.print(WiFi.localIP());

    Serial.print("\nconnecting...");
    auto clientId ="savadhan-gate-" + GATE_ID;
    while (!client.connect(clientId.c_str())) {
        Serial.print(".");
        delay(1000);
    }

    Serial.println("\nconnected!");

    publish("/heartbeat", "hello");
    subscribe("/coordinator");
}

void messageReceived(String &topic, String &payload) {
    Serial.println("incoming: " + topic + " - " + payload);
    cmdStr = payload.c_str();
}

void setup() {
    Serial.begin(115200);

    pinMode(offpin, OUTPUT);
    pinMode(onpin, OUTPUT);
    digitalWrite(offpin, LOW);
    digitalWrite(onpin, LOW);

    pinMode(A_pin, INPUT_PULLDOWN);
    attachInterrupt(A_pin, on_A, RISING);
    pinMode(B_pin, INPUT_PULLDOWN);
    attachInterrupt(B_pin, on_B, RISING);
    pinMode(C_pin, INPUT_PULLDOWN);
    attachInterrupt(C_pin, on_C, RISING);
    pinMode(D_pin, INPUT_PULLDOWN);
    attachInterrupt(D_pin, on_D, RISING);
    pinMode(switch_pin, INPUT_PULLUP);
    attachInterrupt(switch_pin, on_key, CHANGE);

    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASS);

    client.begin(MQTT_CONTROLLER_IP, net);
    client.onMessage(messageReceived);
    connect();

    pinMode(remote_enable_pin, OUTPUT);
    digitalWrite(remote_enable_pin, LOW);
    delay(500);
    digitalWrite(remote_enable_pin, HIGH);
}

unsigned long lastMillis = 0;
int servoStatus = 0;
int dcStatus = 0;

void loop() {
    if (!client.connected()) {   
        connect();    
    } else {
        client.loop();
        delay(10);  // <- fixes some issues with WiFi stability
    }
    
    if (cmdStr == "dc_on") {
        Serial.println("Processing dc_on");
        dcStatus = 1;
        digitalWrite(onpin, HIGH);
        delay(1500);
        digitalWrite(onpin, LOW);
    } else if (cmdStr == "dc_off") {
        Serial.println("Processing dc_off");
        dcStatus = 0;
        digitalWrite(offpin, HIGH);
        delay(1500);
        digitalWrite(offpin, LOW);
    } else if (cmdStr != "") {
        Serial.print("Invalid cmd: "); Serial.println(cmdStr.c_str());
        cmdStr = "";
    }
    if (cmdStr != "") {
        Serial.print("Acknowledging "); Serial.println(cmdStr.c_str());
        publish("/coordinator_ack", cmdStr);
        cmdStr = "";
    }
    if (keyCmd != "") {
        Serial.print("Getting keypress: "); Serial.println(keyCmd.c_str());
        publish("/coordinator_keypress", keyCmd);
        keyCmd = "";
    }

    auto millisNow = millis();
    if (millisNow - lastMillis > 3000) {
        std::ostringstream os;
        os << "{"
            << "\"dcStatus\" : " << dcStatus << ", "
            << "\"ipAddress\": \"" << WiFi.localIP().toString().c_str() << "\""
            << "}";
        auto status = os.str();
        publish("/heartbeat", status.c_str());
        lastMillis = millisNow;
    }    
}