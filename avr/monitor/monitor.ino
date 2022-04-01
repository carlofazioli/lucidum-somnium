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
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
}

int dtEMA = 1000;
unsigned long t_prev = 0;
unsigned long t_curr = 0;
float alphaDT = 0.5;   // alpha~1 => keep hardly any history
float hr = 0;

void loop() {
  if(newWaveformStart()) {
    t_curr = millis();
    dtEMA *= (1-alphaDT);
    dtEMA += alphaDT * (t_curr - t_prev);
    hr = (float) 60 * 1000 / dtEMA;
    t_prev = t_curr;
  }
  Serial.print(',');
  Serial.println(hr);
}

/*
 * After measuring the photodiode signal, we need to identify the 
 * period in order to compute the HR.  To identify the period, we
 * track a Exponentially Weighted Moving Average of the IR signal.
 * Each time the IR signal crosses from below to above its EWMA,
 * we identify the start of a new waveform and note the delta_t 
 * since the previous start.  For EWMA details, see:
 * https://en.wikipedia.org/wiki/Moving_average#Exponential_moving_average
 * 
 * Since we need to poll multiple sensors in the loop(), we can't 
 * simply encapsulate the entire delta_t calculation into one 
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
float bpm = 65.0;
float ir_mean = 405.0;
float ir_ampl = 25.0;
float ir_noise_ampl = 15.0;
float t_sec, ir_reading, ir_noise;
float mockIROpticalSensor() {
  t_sec = (float) millis() / 1000;
  ir_reading = ir_ampl * sin(TWO_PI * bpm * t_sec / 60) + ir_mean;
  ir_noise = ir_noise_ampl * standardNormal();
  return ir_reading + ir_noise;
}

/*
 * To generate normally distributed noise used above, we use the 
 * Box-Muller transform.  Using two values u1, u2 ~ U[0,1], we
 * create two values z1, z2 ~ N(0,1).  Thus, every other call 
 * to this function generates the value pair, and every other call
 * simply returns the cached one.  For details, see:
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
