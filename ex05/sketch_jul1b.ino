// ======== 引脚定义 ========
#define TOUCH_PIN 4   // T0 (GPIO4)
#define LED_PIN   2   // 板载LED (GPIO2)

// ======== 触摸阈值 ========
int threshold = 400;   // 根据串口监视器调整

// ======== 档位相关 ========
int speedLevel = 1;                  // 当前档位 (1,2,3)
const int delayTimes[3] = {20, 10, 5};  // 档位1、2、3对应的每步延时(ms)
// 延时越小，呼吸越快

// ======== 触摸状态变量（用于边缘检测和防抖） ========
bool lastTouchState = false;         // 上一次触摸状态
unsigned long lastTouchTime = 0;     // 上次有效触发时间

// ======== 辅助函数：触摸处理（边沿检测 + 防抖 + 档位切换） ========
void handleTouch() {
  int touchValue = touchRead(TOUCH_PIN);
  bool currentTouch = (touchValue < threshold);

  // 上升沿检测：从未触摸到触摸
  if (currentTouch && !lastTouchState) {
    // 防抖：距离上次触发至少50ms
    if (millis() - lastTouchTime > 50) {
      // 有效触摸，切换档位 1->2->3->1
      speedLevel = (speedLevel % 3) + 1;
      Serial.print("Speed Level: ");
      Serial.println(speedLevel);
      lastTouchTime = millis();    // 更新时间戳
    }
  }
  lastTouchState = currentTouch;    // 更新状态
}

// ======== 初始化 ========
void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(LED_PIN, OUTPUT);
  // 初始状态可以熄灭，呼吸循环会立即开始
}

// ======== 主循环（呼吸灯 + 触摸检测） ========
void loop() {
  // ----- 呼吸上升阶段 (0→255) -----
  for (int pwm = 0; pwm < 255; pwm++) {
    analogWrite(LED_PIN, pwm);
    handleTouch();   // 每个PWM步进都检测触摸，保证响应灵敏
    delay(delayTimes[speedLevel - 1]);  // 根据当前档位延时
  }

  // ----- 呼吸下降阶段 (255→0) -----
  for (int pwm = 255; pwm >= 0; pwm--) {
    analogWrite(LED_PIN, pwm);
    handleTouch();
    delay(delayTimes[speedLevel - 1]);
  }
}