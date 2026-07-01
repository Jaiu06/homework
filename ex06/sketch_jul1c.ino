// ======== 引脚定义 ========
#define LED_A_PIN  4
#define LED_B_PIN  5

// ======== PWM参数 ========
#define PWM_FREQ    5000
#define PWM_RES     8    // 8位分辨率，占空比范围0~255

// ======== 渐变速度控制 ========
int stepDelay = 10;

void setup() {
  // --- 修改开始 ---
  // 旧版 API (已废弃): ledcSetup() + ledcAttachPin()
  // 新版 API (3.x): 使用 ledcAttach() 直接初始化引脚
  // 参数: ledcAttach(引脚, 频率, 分辨率)
  ledcAttach(LED_A_PIN, PWM_FREQ, PWM_RES);
  ledcAttach(LED_B_PIN, PWM_FREQ, PWM_RES);
  // --- 修改结束 ---
}

void loop() {
  // 上升阶段：LED_A渐亮，LED_B渐暗
  for (int duty = 0; duty <= 255; duty++) {
    ledcWrite(LED_A_PIN, duty);
    ledcWrite(LED_B_PIN, 255 - duty);
    // --- 修改结束 ---
    delay(stepDelay);
  }
  
  // 下降阶段：LED_A渐暗，LED_B渐亮
  for (int duty = 255; duty >= 0; duty--) {
    ledcWrite(LED_A_PIN, duty);
    ledcWrite(LED_B_PIN, 255 - duty);
    delay(stepDelay);
  }
}