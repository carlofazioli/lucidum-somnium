/*
 *  Lucid Dreaming BioSensor Monitor and Sensory Stimulation Controller
 * 
 *  Inputs:
 *      Analog voltage from HR sensor
 *      3x analog voltage from accelerometer
 *
 *  Outputs:
 *      PWM LED
 *
 *  Heart Rate measurement code heavily inspired by
 *  https://create.arduino.cc/projecthub/Johan_Ha/from-ky-039-to-heart-rate-0abfca
*/

#include <limits.h>


void setup() {
  Serial.begin(9600);
  randomSeed(0);
}


float HR_g = 70.0;
float tSinceLastMovement = 0.0;
unsigned long millisPrev_g = 0UL;
unsigned long millisCurr_g = 0UL;
unsigned long tSinceLastMovement_ms = 0UL;

void loop() {
  HR_g = updateHR(HR_g);
  tSinceLastMovement_ms = updateTSinceLastMovement(tSinceLastMovement_ms);
  Serial.print(HR_g);  
  Serial.print(',');
  Serial.print(tSinceLastMovement_ms / 1000);
  Serial.println();
}

float updateTSinceLastMovement(float t) {
  unsigned long start = millis();
  float acc = 300 * accelerationSensorIntervalMax();
  if(acc > 290) {
    t = 0;
  } else {
    t += millis() - start;
  }
  return t;
}

/*
 * Here we poll the HR.  To do so, we check if a new wave
 * form has started, and if it has, we compute the HR from
 * the time delta since the last waveform.  We smooth with
 * an EWMA.  See the link later for details about EWMA.  
 * 
 * Relevant human HR is in [30, 180], implying a deltaT
 * in the range [330, 2000].  DeltaTs below this range
 * are spurious and are ignored.  
 * 
 */
float updateHR(float hr) {
  float HRAlpha = 0.8;
  unsigned int deltaT = 0;
  unsigned int deltaTMin = 333;
  if(newWaveformStart()) {
    millisPrev_g = millisCurr_g;
    millisCurr_g = millis();
    deltaT = millisCurr_g - millisPrev_g;
    if(deltaTMin < deltaT) {
      hr *= (1 - HRAlpha);
      hr += HRAlpha * 60 * 1000 / deltaT;
    }
  }
  return hr;
}

/*
 * After measuring the photoresistor signal, we need to identify the 
 * period in order to compute the HR.  To identify the period, we
 * track a Exponentially Weighted Moving Average of the IR signal.
 * Each time the IR signal crosses from below to above its EWMA,
 * we identify the start of a new waveform and note the deltaT 
 * since the previous start.  For EWMA details, see:
 * https://en.wikipedia.org/wiki/Moving_average#Exponential_moving_average
 * 
 * Since we need to poll multiple sensors in the loop(), we can't 
 * simply encapsulate the entire deltaT calculation into one 
 * function.  So this function outputs a 1 when a new waveform is
 * detected and a 0 otherwise.  
 */

float IRPrev_g = 0.0;
float IRCurr_g = 0.0;
float IREWMA_g = 0;
byte newWaveformStart() {
  float IRAlpha = 0.01;  // alpha~0 => keep a lot of EWMA history
  IRPrev_g = IRCurr_g;
  IRCurr_g = IROpticalSensorTimeAvg();
  IREWMA_g *= (1 - IRAlpha);
  IREWMA_g += IRAlpha * IRPrev_g;
  Serial.print(IRCurr_g);
  Serial.print(',');
  Serial.print(IREWMA_g);
  Serial.print(',');
  return (IRPrev_g < IREWMA_g) & (IREWMA_g <= IRCurr_g);
}

/*
 * The physical photoresistor will be noisy, so we want to capture readings
 * that are time-averaged over some interval of width dt.  We use the 
 * do-while construct to ensure that at least 1 measurement is captured,
 * avoiding any divide-by-zero problems in the return value.  
 */
float IROpticalSensorTimeAvg() {
  unsigned int dt = 50;
  float sum = 0;
  unsigned long n = 0;
  unsigned long now;
  unsigned long start = millis();
  do {
    sum += mockIROpticalSensor();
    n++;
    now = millis();
  }
  while(now - start < dt);
  return sum / n;
}

/*
 * In the absense of a physcial IR sensor, we can create mock data.
 * We select a fixed mock BPM of 65.  We "read" the value of the
 * optical sensor as a sine wave function of (real) time at the mock 
 * BPM, and then add some normally distributed noise.  This will be
 * replaced by analogRead(IROpticalSensorPin).
 */
float mockIROpticalSensor() {
  float BPM[] = {65.0, 44.0, 92.0};
  float mockIRMean = 405.0;
  float mockIRAmpl = 25.0;
  float mockIRNoiseAmpl = 15.0;
  float tSec = (float) millis() / 1000;
  byte BPMIdx = ((int) tSec / 60) % 3;
  float mockIRReading = mockIRAmpl * sin(TWO_PI * BPM[BPMIdx] * tSec / 60) + mockIRMean;
  float mockIRNoiseReading = mockIRNoiseAmpl * standardNormalVariate();
  return mockIRReading + mockIRNoiseReading;
}

float accelerationSensorIntervalMax() {
  byte arrayLen = 3;
  float accel[arrayLen];
  unsigned int dt = 50;
  float accelMax = -1.0;
  float mag;
  unsigned long now;
  unsigned long start = millis();
  do {
    mockAccelerometerSensor(accel, arrayLen);
    mag = magnitude(accel, arrayLen);
    accelMax = accelMax < mag ? mag : accelMax;
    now = millis();
  } while(now - start < dt);
  return accelMax;
}

float magnitude(float * vec, byte arrayLen) {
  float m = 0.0;
  for(byte i = 0; i < arrayLen; i++) {
    m += vec[i]*vec[i];
  }
  return sqrt(m);
}

void mockAccelerometerSensor(float * targetArray, byte arrayLen) {
  for(byte i = 0; i < arrayLen; i++) {
    targetArray[i] = normalVariate(0.0, 0.2);
  }
}

/*
 * To generate normally distributed noise used above, we use the 
 * Box-Muller transform.  Using two values u1, u2 ~ U[0,1], we
 * create two values z1, z2 ~ N(0,1).  Thus, every other call 
 * to this function generates the value pair, and every other call
 * simply returns the second cached one.  For details, see:
 * 
 * https://en.wikipedia.org/wiki/Box%E2%80%93Muller_transform
 */
byte needNewNormal_g = 1;
float z2_g;
float standardNormalVariate() {
  if(needNewNormal_g) {
    float u1 = (float) random(1, LONG_MAX) / LONG_MAX;
    float u2 = (float) random(1, LONG_MAX) / LONG_MAX;
    float z1 = sqrt(-2*log(u1))*cos(TWO_PI*u2);
    z2_g = sqrt(-2*log(u1))*sin(TWO_PI*u2);
    needNewNormal_g = 0;
    return z1;
  } else {
    needNewNormal_g = 1;
    return z2_g;
  }
}

float normalVariate(float mean, float std) {
  return std * standardNormalVariate() + mean;
}
