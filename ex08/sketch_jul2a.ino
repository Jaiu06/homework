#include <WiFi.h>
#include <WebServer.h>

// ======== WiFi 配置 ========
const char* ssid = "iqoo";
const char* password = "wangziyin";

// ======== 硬件引脚 ========
const int TOUCH_PIN = 4;   // T0 (GPIO4)
const int LED_PIN   = 2;   // 板载 LED (GPIO2)

// ======== 触摸阈值 ========
int threshold = 400;   // 根据实际调整

// ======== 系统状态变量 ========
bool armed = false;          // 布防状态
bool alarmTriggered = false; // 报警触发标志

// ======== LED 闪烁控制 ========
unsigned long lastFlashTime = 0;
const unsigned long flashInterval = 200; // 200ms 翻转一次，高频闪烁

// ======== Web 服务器 ========
WebServer server(80);

// ----- 检测触摸 -----
bool isTouched() {
  int val = touchRead(TOUCH_PIN);
  return (val < threshold);
}

// ----- LED 闪烁处理（非阻塞） -----
void handleLED() {
  if (alarmTriggered) {
    unsigned long now = millis();
    if (now - lastFlashTime >= flashInterval) {
      lastFlashTime = now;
      digitalWrite(LED_PIN, !digitalRead(LED_PIN)); // 翻转
    }
  } else {
    digitalWrite(LED_PIN, LOW); // 熄灭
  }
}

// ======== 构建 HTML 页面 ========
String buildPage() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html lang="zh-CN">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>安防报警系统</title>
  <style>
    body { font-family: Arial, sans-serif; text-align: center; margin-top: 50px; }
    .container { max-width: 400px; margin: 0 auto; }
    .status { font-size: 1.8em; margin: 20px 0; padding: 10px; border-radius: 10px; }
    .status-armed { background-color: #ffeb3b; }
    .status-disarmed { background-color: #8bc34a; }
    .status-alarm { background-color: #f44336; color: white; animation: blink 0.5s infinite; }
    @keyframes blink { 0%{opacity:1;} 50%{opacity:0.2;} 100%{opacity:1;} }
    .btn { padding: 15px 30px; margin: 10px; font-size: 1.2em; border: none; border-radius: 8px; cursor: pointer; }
    .btn-arm { background-color: #2196F3; color: white; }
    .btn-disarm { background-color: #FF5722; color: white; }
    .btn-arm:hover { background-color: #1976D2; }
    .btn-disarm:hover { background-color: #D84315; }
  </style>
</head>
<body>
<div class="container">
  <h1>🚨 安防报警系统</h1>
  <div id="statusDisplay" class="status">状态加载中...</div>
  <div>
    <button class="btn btn-arm" onclick="arm()">🔒 布防</button>
    <button class="btn btn-disarm" onclick="disarm()">🔓 撤防</button>
  </div>
  <p style="margin-top:30px;">布防后，触摸传感器触发报警，LED 高频闪烁</p>
</div>
<script>
  function updateStatus(data) {
    const display = document.getElementById('statusDisplay');
    // 移除所有类
    display.className = 'status';
    if (data.alarm) {
      display.textContent = '🚨 报警中！';
      display.classList.add('status-alarm');
    } else if (data.armed) {
      display.textContent = '🔒 布防中（待命）';
      display.classList.add('status-armed');
    } else {
      display.textContent = '🔓 撤防（安全）';
      display.classList.add('status-disarmed');
    }
  }

  function fetchStatus() {
    fetch('/status')
      .then(response => response.json())
      .then(data => updateStatus(data))
      .catch(err => console.error('状态获取失败:', err));
  }

  function arm() {
    fetch('/arm', { method: 'POST' })
      .then(() => fetchStatus())
      .catch(err => console.error('布防失败:', err));
  }

  function disarm() {
    fetch('/disarm', { method: 'POST' })
      .then(() => fetchStatus())
      .catch(err => console.error('撤防失败:', err));
  }

  // 页面加载后立即获取状态，并每1秒自动刷新
  window.onload = function() {
    fetchStatus();
    setInterval(fetchStatus, 1000);
  };
</script>
</body>
</html>
)rawliteral";
  return html;
}

// ======== Web 路由处理 ========
void handleRoot() {
  server.send(200, "text/html; charset=UTF-8", buildPage());
}

void handleStatus() {
  String json = "{";
  json += "\"armed\":" + String(armed ? "true" : "false") + ",";
  json += "\"alarm\":" + String(alarmTriggered ? "true" : "false");
  json += "}";
  server.send(200, "application/json", json);
}

void handleArm() {
  armed = true;
  alarmTriggered = false;   // 重新布防时清除之前的报警状态
  digitalWrite(LED_PIN, LOW);
  server.send(200, "text/plain", "OK");
}

void handleDisarm() {
  armed = false;
  alarmTriggered = false;
  digitalWrite(LED_PIN, LOW);
  server.send(200, "text/plain", "OK");
}

// ======== 初始化 ========
void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

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
  server.on("/status", handleStatus);
  server.on("/arm", HTTP_POST, handleArm);
  server.on("/disarm", HTTP_POST, handleDisarm);

  server.begin();
  Serial.println("Web 服务器已启动");
}

// ======== 主循环 ========
void loop() {
  server.handleClient();   // 处理 Web 请求

  // ------ 触摸检测逻辑 ------
  // 仅在布防状态且未报警时才检测触摸
  if (armed && !alarmTriggered) {
    if (isTouched()) {
      alarmTriggered = true;         // 触发报警
      lastFlashTime = millis();      // 重置闪烁计时器，让LED立即开始闪烁
    }
  }

  // ------ LED 闪烁控制 ------
  handleLED();

  // 微小延时，避免看门狗复位
  delay(10);
}