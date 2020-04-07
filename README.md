# sleepy
IoT device that revolutionizes your sleep in countless ways

## watch the video
https://photos.app.goo.gl/mEqhKrbQaX6vw3YB7

## important features:

### fan operation
Personally, I'd always turn on the fan in my room before going to bed but then wake up feeling extremely cold -- too cold to leave my blanket. So, with this device, I can turn off my room's fan with a single tap on my phone. Accomplished using an Arduino Uno, an AC Servo motor and a handmade device which the motor controls in order to turn off the fan. 

### sleep time tracking
Using a microphone connected to the ESP32, the device uses a 3-clap signal to indicate that a user is going to sleep or has woken up. A quick sound/light display follows recognition of the signal and then this data is stored in a SQL database using Flask such that users can easily access their sleep times for many days in the past if desired.

### sound/light/temperature tracking
As an added feature, the ESP32 also has connected a thermoresistor and photoresistor (in addition to the microphone) for recording sound/light/temperature levels (or more accurately disturbances) throughout the night. Data is again stored in a SQL database.

### wake up light
Alarms are annoying. Even an alarm clock is suboptimal in that seeing that time on the clock can make one anxious. So, this device has a simple red light signal which the user can set to turn on at a given time, or even a set amount of hours after they fall asleep. That way, if you wake up and you don't see the light, you can go back to sleep peacefully without worrying about how many hours you've gotten so far or what not. And, when you do wake up on your own terms and see that red light, you know it's time to start your day.
