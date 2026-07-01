// 定义触摸引脚 (T0对应GPIO4)
#define TOUCH_PIN 4
// 定义LED引脚 (ESP32 DevKit板载LED通常是GPIO2)
#define LED_PIN 2
// 中断模式设置：0为轮询模式，1为中断模式
#define EXT_ISR_MODE 0   // 使用轮询模式

// 阈值，需要通过串口监视器观察并调整
int threshold = 400; 

// 触摸值
int touchValue;

// ---- 新增：状态变量 ----
bool ledState = false;        // 当前LED状态（初始熄灭）
bool lastTouchState = false;  // 上一次触摸是否有效（true=触摸中）

void setup() {
  Serial.begin(115200);
  delay(1000); // 等待串口稳定

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW); // 初始熄灭
}

void loop() {
  // 读取当前触摸值
  touchValue = touchRead(TOUCH_PIN);
  Serial.print("Touch Value: ");
  Serial.println(touchValue);

  // 判断当前是否触摸（值小于阈值视为触摸）
  bool currentTouch = (touchValue < threshold);

  // ---- 边缘检测：检测到“从未触摸到触摸”的上升沿 ----
  if (currentTouch && !lastTouchState) {
    // 这是一个有效的按下瞬间

    // ---- 软件防抖：短暂延迟后再次确认 ----
    delay(50);  // 跳过抖动期
    // 重新读取触摸值进行确认（可选，但这里为了简单，直接认为有效）
    // 当然也可以再次读取并判断，但简单起见，直接翻转并更新状态

    // 翻转LED状态
    ledState = !ledState;
    digitalWrite(LED_PIN, ledState ? HIGH : LOW);

    // 更新上次状态为当前触摸状态（此时为true）
    lastTouchState = true;

    // 打印当前LED状态
    Serial.print("LED toggled to: ");
    Serial.println(ledState ? "ON" : "OFF");
  } 
  else {
    // 如果没有发生边沿变化，更新上次触摸状态
    // 注意：只有在非抖动区间才更新，但此处我们仅在确认后更新
    // 如果当前状态与上次不同（即松开），也要更新lastTouchState，以便下一次检测
    if (!currentTouch && lastTouchState) {
      // 松开触摸，更新状态，但不要翻转LED
      lastTouchState = false;
    }
  }

  delay(100); // 适当的轮询间隔，避免串口刷屏
}