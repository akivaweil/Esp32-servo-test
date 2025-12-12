#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include "config/Config.h"
#include "WebServer.h"
#include "Ultrasonic_Sensor.h"
#include "TimeOfFlight_Sensor.h"
#include "Paint_Motor_Controller.h"

//╔═══╗ ════════════════════════════════════════════════════════════════ ╔═══╗
//║ 🌐 WEB SERVER                                                         ║
//╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝

AsyncWebServer server(80);
bool webServerInitialized = false;

// External variables from main.cpp
extern bool ultrasonicActive;
extern bool tofActive;
extern float distanceSum;
extern unsigned int readingCount;
extern float tofDistanceSum;
extern unsigned int tofReadingCount;
extern float lastKnownToFDistance;
extern bool movementActive;
extern bool sequenceComplete;

// HTML Dashboard
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32 Dashboard</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }
        
        body {
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, Oxygen, Ubuntu, Cantarell, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            min-height: 100vh;
            padding: 20px;
            color: #333;
        }
        
        .container {
            max-width: 1200px;
            margin: 0 auto;
        }
        
        .header {
            text-align: center;
            color: white;
            margin-bottom: 30px;
        }
        
        .header h1 {
            font-size: 2.5rem;
            margin-bottom: 10px;
            text-shadow: 2px 2px 4px rgba(0,0,0,0.2);
        }
        
        .header p {
            font-size: 1.1rem;
            opacity: 0.9;
        }
        
        .grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
            gap: 20px;
            margin-bottom: 20px;
        }
        
        .card {
            background: white;
            border-radius: 16px;
            padding: 25px;
            box-shadow: 0 10px 30px rgba(0,0,0,0.2);
            transition: transform 0.3s ease, box-shadow 0.3s ease;
        }
        
        .card:hover {
            transform: translateY(-5px);
            box-shadow: 0 15px 40px rgba(0,0,0,0.3);
        }
        
        .card-title {
            font-size: 1.2rem;
            font-weight: 600;
            margin-bottom: 15px;
            color: #667eea;
            display: flex;
            align-items: center;
            gap: 10px;
        }
        
        .card-title::before {
            content: '';
            width: 4px;
            height: 20px;
            background: linear-gradient(135deg, #667eea, #764ba2);
            border-radius: 2px;
        }
        
        .value {
            font-size: 3rem;
            font-weight: 700;
            color: #333;
            margin: 15px 0;
            text-align: center;
        }
        
        .unit {
            font-size: 1.2rem;
            color: #666;
            margin-left: 5px;
        }
        
        .status {
            display: inline-block;
            padding: 8px 16px;
            border-radius: 20px;
            font-size: 0.9rem;
            font-weight: 600;
            margin-top: 10px;
        }
        
        .status.active {
            background: #10b981;
            color: white;
        }
        
        .status.inactive {
            background: #ef4444;
            color: white;
        }
        
        .status.running {
            background: #3b82f6;
            color: white;
        }
        
        .status.stopped {
            background: #6b7280;
            color: white;
        }
        
        .controls {
            display: flex;
            gap: 10px;
            margin-top: 20px;
            flex-wrap: wrap;
        }
        
        .btn {
            flex: 1;
            min-width: 100px;
            padding: 12px 24px;
            border: none;
            border-radius: 8px;
            font-size: 1rem;
            font-weight: 600;
            cursor: pointer;
            transition: all 0.3s ease;
            text-transform: uppercase;
            letter-spacing: 0.5px;
        }
        
        .btn-primary {
            background: linear-gradient(135deg, #667eea, #764ba2);
            color: white;
        }
        
        .btn-primary:hover {
            transform: scale(1.05);
            box-shadow: 0 5px 15px rgba(102, 126, 234, 0.4);
        }
        
        .btn-danger {
            background: #ef4444;
            color: white;
        }
        
        .btn-danger:hover {
            transform: scale(1.05);
            box-shadow: 0 5px 15px rgba(239, 68, 68, 0.4);
        }
        
        .btn:active {
            transform: scale(0.95);
        }
        
        .info-row {
            display: flex;
            justify-content: space-between;
            padding: 8px 0;
            border-bottom: 1px solid #e5e7eb;
        }
        
        .info-row:last-child {
            border-bottom: none;
        }
        
        .info-label {
            color: #666;
            font-weight: 500;
        }
        
        .info-value {
            color: #333;
            font-weight: 600;
        }
        
        .wifi-status {
            display: flex;
            align-items: center;
            gap: 8px;
        }
        
        .wifi-icon {
            width: 12px;
            height: 12px;
            border-radius: 50%;
            background: #10b981;
            animation: pulse 2s infinite;
        }
        
        @keyframes pulse {
            0%, 100% { opacity: 1; }
            50% { opacity: 0.5; }
        }
        
        @media (max-width: 768px) {
            .header h1 {
                font-size: 2rem;
            }
            
            .grid {
                grid-template-columns: 1fr;
            }
            
            .value {
                font-size: 2.5rem;
            }
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>🚀 ESP32 Dashboard</h1>
            <p>Real-time Sensor & Motor Monitoring</p>
        </div>
        
        <div class="grid">
            <div class="card">
                <div class="card-title">📡 Ultrasonic Sensor</div>
                <div class="value">
                    <span id="ultrasonic-value">--</span>
                    <span class="unit">cm</span>
                </div>
                <div>
                    Status: <span id="ultrasonic-status" class="status inactive">Inactive</span>
                </div>
                <div class="controls">
                    <button class="btn btn-primary" onclick="startUltrasonic()">Start</button>
                    <button class="btn btn-danger" onclick="stopSensors()">Stop</button>
                </div>
            </div>
            
            <div class="card">
                <div class="card-title">🎯 Time of Flight Sensor</div>
                <div class="value">
                    <span id="tof-value">--</span>
                    <span class="unit">cm</span>
                </div>
                <div>
                    Status: <span id="tof-status" class="status inactive">Inactive</span>
                </div>
                <div class="controls">
                    <button class="btn btn-primary" onclick="startToF()">Start</button>
                    <button class="btn btn-danger" onclick="stopSensors()">Stop</button>
                </div>
            </div>
            
            <div class="card">
                <div class="card-title">⚙️ Stepper Motor</div>
                <div class="info-row">
                    <span class="info-label">Status:</span>
                    <span id="motor-status" class="status stopped">Stopped</span>
                </div>
                <div class="info-row">
                    <span class="info-label">Position:</span>
                    <span class="info-value" id="motor-position">0</span>
                </div>
                <div class="info-row">
                    <span class="info-label">Sequence:</span>
                    <span id="sequence-status" class="status stopped">Active</span>
                </div>
            </div>
            
            <div class="card">
                <div class="card-title">📶 System Info</div>
                <div class="info-row">
                    <span class="info-label">WiFi Status:</span>
                    <span class="wifi-status">
                        <span class="wifi-icon"></span>
                        <span id="wifi-status" class="info-value">Connected</span>
                    </span>
                </div>
                <div class="info-row">
                    <span class="info-label">IP Address:</span>
                    <span class="info-value" id="ip-address">--</span>
                </div>
                <div class="info-row">
                    <span class="info-label">Uptime:</span>
                    <span class="info-value" id="uptime">--</span>
                </div>
            </div>
        </div>
    </div>
    
    <script>
        function updateDashboard() {
            fetch('/api/data')
                .then(response => response.json())
                .then(data => {
                    // Update Ultrasonic
                    if (data.ultrasonic.active) {
                        document.getElementById('ultrasonic-value').textContent = data.ultrasonic.distance.toFixed(2);
                        document.getElementById('ultrasonic-status').textContent = 'Active';
                        document.getElementById('ultrasonic-status').className = 'status active';
                    } else {
                        document.getElementById('ultrasonic-value').textContent = '--';
                        document.getElementById('ultrasonic-status').textContent = 'Inactive';
                        document.getElementById('ultrasonic-status').className = 'status inactive';
                    }
                    
                    // Update ToF
                    if (data.tof.active) {
                        document.getElementById('tof-value').textContent = data.tof.distance.toFixed(2);
                        document.getElementById('tof-status').textContent = 'Active';
                        document.getElementById('tof-status').className = 'status active';
                    } else {
                        document.getElementById('tof-value').textContent = '--';
                        document.getElementById('tof-status').textContent = 'Inactive';
                        document.getElementById('tof-status').className = 'status inactive';
                    }
                    
                    // Update Motor
                    if (data.motor.running) {
                        document.getElementById('motor-status').textContent = 'Running';
                        document.getElementById('motor-status').className = 'status running';
                    } else {
                        document.getElementById('motor-status').textContent = 'Stopped';
                        document.getElementById('motor-status').className = 'status stopped';
                    }
                    document.getElementById('motor-position').textContent = data.motor.position;
                    
                    if (data.motor.sequenceComplete) {
                        document.getElementById('sequence-status').textContent = 'Complete';
                        document.getElementById('sequence-status').className = 'status stopped';
                    } else {
                        document.getElementById('sequence-status').textContent = 'Active';
                        document.getElementById('sequence-status').className = 'status running';
                    }
                    
                    // Update System Info
                    document.getElementById('ip-address').textContent = data.system.ip;
                    document.getElementById('uptime').textContent = formatUptime(data.system.uptime);
                })
                .catch(error => console.error('Error:', error));
        }
        
        function formatUptime(ms) {
            const seconds = Math.floor(ms / 1000);
            const minutes = Math.floor(seconds / 60);
            const hours = Math.floor(minutes / 60);
            const days = Math.floor(hours / 24);
            
            if (days > 0) return `${days}d ${hours % 24}h`;
            if (hours > 0) return `${hours}h ${minutes % 60}m`;
            if (minutes > 0) return `${minutes}m ${seconds % 60}s`;
            return `${seconds}s`;
        }
        
        function startUltrasonic() {
            fetch('/api/command?cmd=ultra')
                .then(() => updateDashboard());
        }
        
        function startToF() {
            fetch('/api/command?cmd=tof')
                .then(() => updateDashboard());
        }
        
        function stopSensors() {
            fetch('/api/command?cmd=stop')
                .then(() => updateDashboard());
        }
        
        // Update dashboard every 500ms
        setInterval(updateDashboard, 500);
        
        // Initial update
        updateDashboard();
    </script>
</body>
</html>
)rawliteral";

void initializeWebServer() {
    if (webServerInitialized) {
        return;
    }
    
    // Wait for WiFi connection
    if (WiFi.status() != WL_CONNECTED) {
        return;
    }
    
    // Serve dashboard
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send_P(200, "text/html", index_html);
    });
    
    // API endpoint for sensor data
    server.on("/api/data", HTTP_GET, [](AsyncWebServerRequest *request){
        String json = "{";
        
        // Ultrasonic data
        float ultrasonicDistance = 0.0;
        if (ultrasonicActive && readingCount > 0) {
            ultrasonicDistance = distanceSum / readingCount;
        }
        json += "\"ultrasonic\":{";
        json += "\"active\":" + String(ultrasonicActive ? "true" : "false") + ",";
        json += "\"distance\":" + String(ultrasonicDistance, 2);
        json += "},";
        
        // ToF data
        float tofDistance = 0.0;
        if (tofActive) {
            if (tofReadingCount > 0) {
                tofDistance = tofDistanceSum / tofReadingCount;
            } else if (lastKnownToFDistance > 0.0) {
                // Use last known value when current reading is zero
                tofDistance = lastKnownToFDistance;
            }
        }
        json += "\"tof\":{";
        json += "\"active\":" + String(tofActive ? "true" : "false") + ",";
        json += "\"distance\":" + String(tofDistance, 2);
        json += "},";
        
        // Motor data
        json += "\"motor\":{";
        json += "\"running\":" + String(isStepperRunning() ? "true" : "false") + ",";
        json += "\"position\":" + String(getStepperPosition()) + ",";
        json += "\"sequenceComplete\":" + String(sequenceComplete ? "true" : "false");
        json += "},";
        
        // System info
        json += "\"system\":{";
        json += "\"ip\":\"" + WiFi.localIP().toString() + "\",";
        json += "\"uptime\":" + String(millis());
        json += "}";
        
        json += "}";
        
        request->send(200, "application/json", json);
    });
    
    // API endpoint for commands
    server.on("/api/command", HTTP_GET, [](AsyncWebServerRequest *request){
        if (request->hasParam("cmd")) {
            String cmd = request->getParam("cmd")->value();
            cmd.toLowerCase();
            
            if (cmd == "ultra") {
                ultrasonicActive = true;
                tofActive = false;
                distanceSum = 0.0;
                readingCount = 0;
                tofDistanceSum = 0.0;
                tofReadingCount = 0;
            } else if (cmd == "tof") {
                tofActive = true;
                ultrasonicActive = false;
                tofDistanceSum = 0.0;
                tofReadingCount = 0;
                distanceSum = 0.0;
                readingCount = 0;
            } else if (cmd == "stop") {
                ultrasonicActive = false;
                tofActive = false;
                distanceSum = 0.0;
                readingCount = 0;
                tofDistanceSum = 0.0;
                tofReadingCount = 0;
            }
            
            request->send(200, "application/json", "{\"status\":\"ok\"}");
        } else {
            request->send(400, "application/json", "{\"error\":\"Missing cmd parameter\"}");
        }
    });
    
    server.begin();
    webServerInitialized = true;
}

void updateWebServer() {
    // Initialize web server once WiFi is connected
    if (!webServerInitialized && WiFi.status() == WL_CONNECTED) {
        initializeWebServer();
    }
}
