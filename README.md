# lucidum-somnium

A project to integrate biosensors and sensory stimulation to induce 
lucid dreaming in the subject.  

## Summary

A suite of biosensors gather data about the sleep state of the wearer.  
Candidate biosensors include:
*   Heart rate monitor
*   Actigraph (motion detector)
*   Respiration rate monitor
*   EOG/EMG: electrooculograph/electromyograph
*   EEG: electroencephalograph
*   Video images

The biosensor data is collected in real time by an AVR microcontroller; 
e.g. Arduino.

After preprocessing the data on the AVR, the data is marshalled and sent 
via serial line to monitoring software on a PC.  The monitoring software 
uses the sensor data to identify when the wearer has entered the REM 
sleep stage.  At this time the monitoring software sends a signal to 
back to the AVR to activate the sensory stimulation.  

Various stimulation devices are on/near the subject, and emit signals 
intended to be subtly perceived by the subject during REM sleep and
assist in triggering an episode of lucid dreaming.  Candidate stimulation
devices include:
*   LEDs or other light sources
*   Speaker 
*   Haptic vibrations

## Roadmap

To develop an initial prototype, we scope the biosensors to include only
a HR monitor and actigraph.  

To calibrate the monitoring software, data from a commercially available
sleep monitoring app can be used; e.g. Pillow.  

The initial prototype sensory stimulation can be limited to flashing an
LED.  

# Sensors

## Heart Rate Monitor

A number of existing Arduino projects use a KY-039 sensor to indirectly
measure heart rate.  See 
[this example](https://create.arduino.cc/projecthub/Johan_Ha/from-ky-039-to-heart-rate-0abfca).

The KY-039 is perhaps no longer readily available, but it is a 
particularly simple device and can be replicated with basic components.

## Actigraph

A cheapie accelerometer is sufficient, as it will be used to detect body
motion from tossing/turning in bed.  A low resolution, 2-d device should
do the trick.  

## Flashing LED

Probably a 'warm' color like orange or red.  

# Monitoring Software

The monitoring software is responsible for ingesting the biosensor data
and using it to determine when the subject is in REM sleep.  

## Initial Prototype

The initial prototype monitoring software can be run entirely on the
ARV.  

### Heart Rate Monitoring

Other HR sensor projects indicate that the signal from a KY-039 is 
relatively noisy.  The raw signal is time-averaged.  This smoothed
signal can be used to identify peaks, and their period.  This can
all be done onboard the AVR.  

### Actigraph

The actigraph is simply identifying spikes in the acceleromter data,
and keeping track of the amount of time since the last one.  

### REM Threshold

Initially, the decision function can monitor for a sufficiently low
HR and a sufficiently large time elapsed since last actigraphy.  

### Sensory Stimulator

The prototype sensory stimulation can be a blinking pattern for an
LED mounted near the subject's eyes.  
