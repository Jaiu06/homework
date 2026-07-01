#include <WiFi.h>
#include <WebServer.h>

// ======== WiFi 配置 ========
const char* ssid = "iqoo";
const char* password = "wangziyin";

// ======== 触摸引脚 ========
const int TOUCH_PIN = 4;   // T0 (GPIO4)

// ======== Web 服务器 ========
WebServer server(80);

// ======== 构建 HTML 页面（带 AJAX 轮询） ========
String buildPage() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html lang="zh-CN">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>实时传感器仪表盘</title>
  <style>
    body { font-family: Arial, sans-serif; text-align: center; margin-top: 50px; background-color: #f0f4f8; }
    .container { max-width: 600px; margin: 0 auto; background: white; padding: 30px; border-radius: 20px; box-shadow: 0 10px 30px rgba(0,0,0,0.1); }
    h1 { color: #333; }
    .sensor-value {
      font-size: 5em;
      font-weight: bold;
      color: #2196F3;
      padding: 20px;
      background: #e3f2fd;
      border-radius: 15px;
      margin: 20px 0;
      transition: background 0.3s;
    }
    .sensor-value.low { background: #ffcdd2; color: #d32f2f; }
    .unit { font-size: 0.3em; color: #555; }
    .status { font-size: 1.2em; color: #666; }
    .timestamp { font-size: 0.8em; color: #999; margin-top: 10px; }
  </style>
</head>
<body>
<div class="container">
  <h1>📊 触摸传感器实时数值</h1>
  <div class="sensor-value" id="sensorValue">--</div>
  <div class="status" id="statusText">正在读取数据...</div>
  <div class="timestamp" id="timestamp">最后更新：--</div>
  <p style="margin-top:20px; color:#888;">手靠近时数值减小，离开恢复</p>
</div>

<script>
  function updateSensor() {
    fetch('/data')
      .then(response => {
        if (!response.ok) throw new Error('网络错误');
        return response.json();
      })
      .then(data => {
        const value = data.value;
        const display = document.getElementById('sensorValue');
        display.textContent = value;

        // 根据数值大小改变颜色（可自定义阈值）
        if (value < 30) {
          display.className = 'sensor-value low';   // 接近触摸
        } else {
          display.className = 'sensor-value';
        }

        document.getElementById('statusText').textContent = '实时数据';
        document.getElementById('timestamp').textContent = '最后更新：' + new Date().toLocaleTimeString();
      })
      .catch(err => {
        document.getElementById('statusText').textContent = '⚠️ 数据加载失败，请检查网络';
        console.error('Fetch错误:', err);
      });
  }

  // 页面加载后立即获取，然后每300ms轮询一次
  window.onload = function() {
    updateSensor();
    setInterval(updateSensor, 300);  // 300ms 刷新一次，可调整
  };
</script>
</body>
</html>
)rawliteral";
  return html;
}

// ======== 根路径：返回 HTML 页面 ========
void handleRoot() {
  server.send(200, "text/html; charset=UTF-8", buildPage());
}

// ======== 数据接口：返回 JSON ========
void handleData() {
  int touchValue = touchRead(TOUCH_PIN);
  String json = "{ \"value\": " + String(touchValue) + " }";
  server.send(200, "application/json", json);
}

// ======== 初始化 ========
void setup() {
  Serial.begin(115200);

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
  server.on("/data", handleData);

  server.begin();
  Serial.println("Web 服务器已启动");
}

// ======== 主循环 ========
void loop() {
  server.handleClient();
  // 不需要任何延时，服务器会一直处理请求
}