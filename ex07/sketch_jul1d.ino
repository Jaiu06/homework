#include <WiFi.h>
#include <WebServer.h>

// ======== WiFi 配置 ========
const char* ssid = "iqoo";
const char* password = "wangziyin";

// ======== LED 引脚 ========
const int LED_PIN = 2;   // GPIO2（板载 LED）

// ======== PWM 参数（ESP32 3.x） ========
#define PWM_FREQ  5000
#define PWM_RES   8

// ======== Web 服务器 ========
WebServer server(80);

// ======== 生成 HTML 页面 ========
String makePage() {
  int currentDuty = ledcRead(LED_PIN);   // 读取当前占空比
  
  String html = R"rawliteral(
<!DOCTYPE html>
<html lang="zh-CN">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>无极调光器 + 开关</title>
  <style>
    body { font-family: Arial, sans-serif; text-align: center; margin-top: 50px; }
    .container { max-width: 400px; margin: 0 auto; }
    .slider { width: 100%; height: 20px; }
    .brightness-value { font-size: 2em; font-weight: bold; color: #2196F3; }
    .btn-group { margin-top: 20px; }
    .btn { padding: 10px 30px; margin: 0 10px; font-size: 1em; border: none; border-radius: 5px; cursor: pointer; }
    .btn-on { background-color: #4CAF50; color: white; }
    .btn-off { background-color: #f44336; color: white; }
    .btn-on:hover { background-color: #45a049; }
    .btn-off:hover { background-color: #d32f2f; }
  </style>
</head>
<body>
  <div class="container">
    <h1>🎛️ 无极调光器</h1>
    <p>拖动滑块调节 LED 亮度</p>
    <input type="range" min="0" max="255" value=")rawliteral" + String(currentDuty) + R"rawliteral(" 
           class="slider" id="brightnessSlider" oninput="updateBrightness(this.value)">
    <p>当前亮度：<span class="brightness-value" id="brightnessDisplay">)rawliteral" + String(currentDuty) + R"rawliteral(</span></p>
    
    <div class="btn-group">
      <button class="btn btn-on" onclick="setOn()">💡 点亮</button>
      <button class="btn btn-off" onclick="setOff()">⏻ 熄灭</button>
    </div>
  </div>

  <script>
    // 滑块滑动时调用
    function updateBrightness(val) {
      document.getElementById('brightnessDisplay').textContent = val;
      fetch('/set?value=' + val)
        .catch(err => console.error('网络错误:', err));
    }

    // 点亮：设置为255
    function setOn() {
      fetch('/on')
        .then(() => {
          document.getElementById('brightnessSlider').value = 255;
          document.getElementById('brightnessDisplay').textContent = '255';
        })
        .catch(err => console.error('网络错误:', err));
    }

    // 熄灭：设置为0
    function setOff() {
      fetch('/off')
        .then(() => {
          document.getElementById('brightnessSlider').value = 0;
          document.getElementById('brightnessDisplay').textContent = '0';
        })
        .catch(err => console.error('网络错误:', err));
    }
  </script>
</body>
</html>
)rawliteral";
  return html;
}

// ======== 根路径 ========
void handleRoot() {
  server.send(200, "text/html; charset=UTF-8", makePage());
}

// ======== 设置任意亮度 ========
void handleSet() {
  if (server.hasArg("value")) {
    int duty = server.arg("value").toInt();
    if (duty < 0) duty = 0;
    if (duty > 255) duty = 255;
    ledcWrite(LED_PIN, duty);
    server.send(200, "text/plain", "OK");
  } else {
    server.send(400, "text/plain", "Missing 'value'");
  }
}

// ======== 点亮（固定255） ========
void handleOn() {
  ledcWrite(LED_PIN, 255);
  server.send(200, "text/plain", "OK");   // 返回成功，不重定向
}

// ======== 熄灭（固定0） ========
void handleOff() {
  ledcWrite(LED_PIN, 0);
  server.send(200, "text/plain", "OK");
}

void setup() {
  Serial.begin(115200);

  // 初始化 PWM（ESP32 3.x）
  ledcAttach(LED_PIN, PWM_FREQ, PWM_RES);
  ledcWrite(LED_PIN, 0);

  // 连接 WiFi
  WiFi.begin(ssid, password);
  Serial.print("连接 WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n连接成功！");
  Serial.print("访问地址: http://");
  Serial.println(WiFi.localIP());

  // 注册路由
  server.on("/", handleRoot);
  server.on("/set", handleSet);
  server.on("/on", handleOn);
  server.on("/off", handleOff);

  server.begin();
  Serial.println("Web 服务器已启动");
}

void loop() {
  server.handleClient();
}