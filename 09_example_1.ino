// Arduino pin assignment
#define PIN_LED  9
#define PIN_TRIG 12
#define PIN_ECHO 13

// configurable parameters
#define SND_VEL 346.0     // sound velocity at 24 celsius degree (unit: m/sec)
#define INTERVAL 25       // sampling interval (unit: msec)
#define PULSE_DURATION 10 // ultra-sound Pulse Duration (unit: usec)
#define _DIST_MIN 100     // minimum distance to be measured (unit: mm)
#define _DIST_MAX 300     // maximum distance to be measured (unit: mm)

#define TIMEOUT ((INTERVAL / 2) * 1000.0) // maximum echo waiting time (unit: usec)
#define SCALE (0.001 * 0.5 * SND_VEL)     // coefficent to convert duration to distance

#define N 30

// global variables
unsigned long last_sampling_time;   // unit: msec
float dist_raw;
float distance_samples[N];
int sample_index = 0;
float dist_ema;                     // EMA distance
#define _EMA_ALPHA 0.5              // EMA weight of new sample (range: 0 to 1)

// Function prototypes
float USS_measure(int TRIG, int ECHO);
float calculate_median();

void setup() {
  // initialize GPIO pins
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  digitalWrite(PIN_TRIG, LOW);

  // initialize serial port
  Serial.begin(57600);
}

void loop() {
  // wait until next sampling time.
  if (millis() < last_sampling_time + INTERVAL)
    return;

  // get a distance reading from the USS
  dist_raw = USS_measure(PIN_TRIG, PIN_ECHO);
  
  distance_samples[sample_index] = dist_raw;
  sample_index = (sample_index + 1) % N; // 인덱스 순환

  float dist_median = calculate_median();

  static float dist_prev = _DIST_MAX; // 이전 거리값 초기화
  dist_ema = dist_raw * _EMA_ALPHA + dist_prev * (1 - _EMA_ALPHA);
  dist_prev = dist_ema;

  if ((dist_raw == 0.0) || (dist_raw > _DIST_MAX) || (dist_raw < _DIST_MIN)) {
      digitalWrite(PIN_LED, 1); // LED OFF
  } else {
      digitalWrite(PIN_LED, 0); // LED ON      
  }

  // output the distance to the serial port
  Serial.print("Min:");   Serial.print(_DIST_MIN);
  Serial.print(",raw:");  Serial.print(dist_raw);
  Serial.print(",ema:");  Serial.print(dist_ema); // EMA 값 출력
  Serial.print(",median:");  Serial.print(dist_median);
  Serial.print(",Max:");  Serial.print(_DIST_MAX);
  Serial.println("");

  // update last sampling time
  last_sampling_time += INTERVAL;
}

// get a distance reading from USS. return value is in millimeter.
float USS_measure(int TRIG, int ECHO) {
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(PULSE_DURATION);
  digitalWrite(TRIG, LOW);
  
  return pulseIn(ECHO, HIGH, TIMEOUT) * SCALE; // unit: mm
}

float calculate_median() {
  float sorted_samples[N];
  for (int i = 0; i < N; i++) {
    sorted_samples[i] = distance_samples[i];
  }

  for (int i = 0; i < N - 1; i++) {
    for (int j = i + 1; j < N; j++) {
      if (sorted_samples[i] > sorted_samples[j]) {
        float temp = sorted_samples[i];
        sorted_samples[i] = sorted_samples[j];
        sorted_samples[j] = temp;
      }
    }
  }

  if (N % 2 == 0) {
    return (sorted_samples[N / 2 - 1] + sorted_samples[N / 2]) / 2.0;
  } else {
    return sorted_samples[N / 2];
  }
}
