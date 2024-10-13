#include <Servo.h>
#include <math.h> // for exp()

#define PIN_SERVO 10

Servo myServo;
unsigned long MOVING_TIME = 3000; // moving time is 3 seconds
unsigned long moveStartTime;
int startAngle = 0; // 시작 각도
int stopAngle  = 180; // 종료 각도

// 시그모이드 함수 구현
float sigmoid(unsigned long x, unsigned long maxTime) {
    float k = 0.003; // 경사 조정
    float x0 = maxTime / 2; // 중앙에서 변화하는 지점
    return 1 / (1 + exp(-k * (x - x0)));
}

void setup() {
    myServo.attach(PIN_SERVO);
    moveStartTime = millis(); // 시작 시간 기록

    myServo.write(startAngle); // 초기 위치 설정
    delay(500);
}

void loop() {
    unsigned long progress = millis() - moveStartTime;

    if (progress <= MOVING_TIME) {
        // 시그모이드 함수를 사용하여 각도 계산
        float sigmoidValue = sigmoid(progress, MOVING_TIME);
        long angle = startAngle + (stopAngle - startAngle) * sigmoidValue; 
        myServo.write(angle); 
    }
}
