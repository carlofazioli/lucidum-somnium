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

void setup() {
  Serial.begin(9600);
  randomSeed(0);
}

void loop() {
  pollHR();
}

// Relevant human HR is in [30, 180], implying a deltaT
// in the range [330, 2000].  deltaT outside this range
// is spurious and is ignored.  
unsigned int deltaTMin = 330;
unsigned int deltaTMax = 2000;
unsigned long millisPrev = 0UL;
unsigned long millisCurr = 0;
unsigned int deltaT = 0;
float HRAlpha = 0.35;
float HRCurr = 0;
float HREWMA = 0.0;
void pollHR() {
  if(newWaveformStart()) {
    millisPrev = millisCurr;
    millisCurr = millis();
    deltaT = millisCurr - millisPrev;
    if((deltaTMin < deltaT) & (deltaT < deltaTMax)) {
      HRCurr = (float) 60 * 1000 / deltaT;
      HREWMA *= (1 - HRAlpha);
      HREWMA += HRAlpha * HRCurr;
    }
  }
  Serial.print(',');
  Serial.println(HREWMA);
}

/*
 * After measuring the photodiode signal, we need to identify the 
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

float IRPrev = 0.0;
float IRCurr = 0.0;
float IREWMA = 0;
float IRAlpha = 0.01;  // alpha~0 => keep a lot of EWMA history
byte newWaveformStart() {
  IRPrev = IRCurr;
  IRCurr = IROpticalSensorTimeAvg();
  IREWMA *= (1 - IRAlpha);
  IREWMA += IRAlpha * IRCurr;
  Serial.print(IRCurr);
  Serial.print(',');
  Serial.print(IREWMA);
  Serial.print(',');
  return (IRPrev < IREWMA) & (IREWMA <= IRCurr);
}

/*
 * The physical photodiode will be noisy, so we want to capture readings
 * that are time-averaged over some interval of width dt.  We use the 
 * do-while construct to ensure that at least 1 measurement is captured,
 * avoiding any divide-by-zero problems in the return value.  
 */
unsigned int dt = 50;
float sum;
unsigned long n, now, start;
float IROpticalSensorTimeAvg() {
  sum = 0;
  n = 0;
  start = millis();
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
 * bpm, and then add some normally distributed noise.  This will be
 * replaced by analogRead(IROpticalSensorPin).
 */
float BPM[] = {65.0, 44.0};
float mockIRMean = 405.0;
float mockIRAmpl = 25.0;
float mockIRNoiseAmpl = 15.0;
float tSec, mockIRReading, mockIRNoiseReading;
byte BPMIdx;
float mockIROpticalSensor() {
  tSec = (float) millis() / 1000;
  // Determine which BPM to use; alternate each minute
  BPMIdx = ((int) tSec / 60) % 2;
  mockIRReading = mockIRAmpl * sin(TWO_PI * BPM[BPMIdx] * tSec / 60) + mockIRMean;
  mockIRNoiseReading = mockIRNoiseAmpl * standardNormal();
  return mockIRReading + mockIRNoiseReading;
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
unsigned long LONG_MAX = 2147483647;
byte needNewNormal = 1;
float u1, u2, z1, z2;
float standardNormal() {
  if(needNewNormal) {
    u1 = (float) random(1, LONG_MAX) / LONG_MAX;
    u2 = (float) random(1, LONG_MAX) / LONG_MAX;
    z1 = sqrt(-2*log(u1))*cos(TWO_PI*u2);
    z2 = sqrt(-2*log(u1))*sin(TWO_PI*u2);
    needNewNormal = 0;
    return z1;
  } else {
    needNewNormal = 1;
    return z2;
  }
}
