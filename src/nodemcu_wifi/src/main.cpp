/* General */
#include <Arduino.h>
#include <stdlib.h>
#include <string.h>
#include <sstream>
#include <unordered_map>

/* Firebase */
#include <ArduinoJson.h>
#include <FirebaseArduino.h>
#include <ESP8266HTTPClient.h>

/* I2C */
#include <Wire.h>
#include <time.h>

/*WiFi Manager*/
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

/*Time Tracking*/
#include <NTPClient.h>
#define NTP_OFFSET   60 * 60      // In seconds
#define NTP_INTERVAL 60 * 1000    // In miliseconds
#define NTP_ADDRESS  "3.mx.pool.ntp.org"

/* Credentials */   
#define ssid "IZZI-8E07"
#define pass "HERNANDEZ2019"
#define FIREBASE_HOST "https://atm-technologies-71af7-default-rtdb.firebaseio.com/"
#define FIREBASE_AUTH "Nv0OOiB8ysKGXI771VgHaCDSavk39W1fX9OpnudP"

using namespace std;

void wifiConnect(void);
void connectFirebase(void);
void receiveI2C(int bytes);
void requestI2C(void);
int getAccount();
bool getNCCode();
void setBalance();
void statusChecked(void);
int32_t& searchOnArray(int32_t array[3][2], bool flag);

int16_t func = 0x00;
int32_t CERO = 0;
int32_t data, currentAcc = 1;

int32_t Cuentas[3][2] = {
    {111222333, 10000},
    {400500600, 500}, {123123123, 500000} };


int32_t NCCodes_M[3][2] = {
    {123456, 1000},
    {102030, 100},
    {101010, 2000}
};

int32_t NCCodes_A[3][2] = {
    {123456, 111222333},
    {102030, 111222333},
    {101010, 123123123}
};

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, NTP_ADDRESS, NTP_OFFSET, NTP_INTERVAL);

void setup() {
    Serial.begin(9600);
    pinMode(LED_BUILTIN, OUTPUT);
    wifiConnect();
    connectFirebase();
    Wire.begin(0x17);
    
    Wire.onReceive(receiveI2C);
    Wire.onRequest(requestI2C);
    timeClient.begin();
}

void loop() {

}

void wifiConnect() {
    digitalWrite(LED_BUILTIN, HIGH);
    Serial.println("Connecting to ");
    Serial.println(ssid);
    
    WiFi.begin(ssid, pass);
    while (WiFi.status() != WL_CONNECTED) 
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected");
    digitalWrite(LED_BUILTIN, LOW);
}

void connectFirebase(void) {
    Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
    if (Firebase.failed()) {
        Serial.println(Firebase.error());
        connectFirebase();
    }
    else {
        Serial.println("Firebase connected");
    }
}

void receiveI2C(int bytes) {
    if (bytes == 1) {
        func = Wire.read();
        if (func == 0x8) {
            //statusChecked();
        }
    } else {
        data = 0x0000;
        for (int i = bytes-1; i >= 0; i--) {
            data |= Wire.read() << (i*8);
        }
        if (func == 0x4) {
            setBalance();
        } else if (func == 0x1) {
            currentAcc = data;
        }
    }
}

void requestI2C(void) {
    if (func == 0x2) {
        currentAcc = searchOnArray(NCCodes_A, true);
        Wire.write(searchOnArray(NCCodes_M, true));
        //getAccount();
    }
    Wire.write(searchOnArray(Cuentas, false));
    //getNCCode();
}

int getAccount(void) {
    std::string _account;
    stringstream ss;
    ss << currentAcc;
    ss >> _account;
    
    std::string path = "Accounts/" + _account + "/Balance";
    int db_account = Firebase.getInt(path.c_str());
    Serial.print("Balance: ");
    Serial.println(db_account);
    if (Firebase.success()) {
        return db_account;
    }
    else {
        return 0;
    }
}

bool getNCCode() {
    return true;
}

void setBalance() {
    searchOnArray(Cuentas, false) = data;
}

void statusChecked(void) {
    int id = Firebase.getInt("Status/Total") + 1;
    Serial.print("Firebase.getInt() status: ");
    Serial.println(Firebase.success());
    unsigned long time = timeClient.getEpochTime();
    DynamicJsonBuffer buffer(1024);
    std::string _account, _time, _id;
    stringstream ss;
    ss << currentAcc;
    ss >> _account;
    ss << id;
    ss >> _id;
    ss << time;
    ss >> _time;
    std::string statusReg = "{\"" + _id + "\":{AccountId\":" + _account + ",\"Time\":" + _time + "}}";
    Firebase.push("Status/", buffer.parse(statusReg.c_str()));
    Serial.println(Firebase.success());
    Firebase.setInt("Status/Total", id);
    Serial.println(Firebase.success());
}

int32_t& searchOnArray(int32_t array[3][2], bool flag) {
    int i;
    int32_t search;
    if (flag) {
        search = data;
    } else {
        search = currentAcc;
    }
    for (i = 0; i < 3; i++) {
        if (array[i][0] == search) {
            return array[i][1];
        }
    }
    return CERO;
}
