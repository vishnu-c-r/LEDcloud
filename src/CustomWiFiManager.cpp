#include "CustomWiFiManager.h"

CustomWiFiManager::CustomWiFiManager(const char *deviceName, AsyncWebServer* asyncServer)
    : hostname(deviceName), server(asyncServer), shouldRun(false)
{
    wifiManager = nullptr;
    if (!server) {
        // If no server provided, create one on port 80
        server = new AsyncWebServer(80);
    }
    wifiManager = new AsyncWiFiManager(server, &dns);
}

CustomWiFiManager::~CustomWiFiManager()
{
    if (wifiManager) {
        delete wifiManager;
    }
    // Do not delete server, as it may be managed elsewhere
}

void CustomWiFiManager::checkWiFiConnection()
{
    if (!isConnected()) {
        begin();
    }
    MDNS.update();
}

void CustomWiFiManager::startTask()
{
    if (!shouldRun) {
        shouldRun = true;
        wifiTicker.attach_ms(1000, [this]() { this->checkWiFiConnection(); });
    }
}

void CustomWiFiManager::stopTask()
{
    if (shouldRun) {
        shouldRun = false;
        wifiTicker.detach();
    }
}

bool CustomWiFiManager::begin()
{
    if (!wifiManager->autoConnect(hostname.c_str())) {
        Serial.println("Failed to connect and hit timeout");
        delay(3000);
        return false;
    }

    // Set hostname in WiFi config
    WiFi.hostname(hostname.c_str());
    
    // Clear any existing mDNS services
    MDNS.end();
    
    // Start mDNS with explicit hostname
    if (MDNS.begin(hostname.c_str())) {
        // Add service advertising
        MDNS.addService("http", "tcp", 80);
        Serial.print("mDNS responder started: http://");
        Serial.print(hostname);
        Serial.println(".local");
    } else {
        Serial.println("Error setting up MDNS responder!");
    }

    Serial.println("WiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.print("Hostname: ");
    Serial.println(hostname);
    return true;
}

void CustomWiFiManager::reset()
{
    wifiManager->resetSettings();
    ESP.restart();
}

bool CustomWiFiManager::isConnected()
{
    return WiFi.status() == WL_CONNECTED;
}

String CustomWiFiManager::getIP()
{
    return WiFi.localIP().toString();
}

String CustomWiFiManager::getHostname()
{
    return hostname;
}

void CustomWiFiManager::setupClientIPLogging(AsyncWebServer* server) {
    if (!server) {
        return;
    }
    
    // Add default handler to log client IP
    server->onRequestBody([](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
        Serial.print("Request from IP: ");
        Serial.println(request->client()->remoteIP().toString());
    });
}