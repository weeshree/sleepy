# sleepy
IoT device that revolutionizes your sleep in countless ways

## important features:

### fan operation
Personally, I'd always turn on the fan in my room before going to bed but then wake up feeling extremely cold -- too cold to leave my blanket. So, with this device, I can turn off my room's fan with a single tap on my phone. Accomplished using an Arduino Uno, an AC Servo motor and a handmade device which the motor controls in order to turn off the fan. 

### sleep time tracking
Using a microphone connected to the ESP32, the device uses a 3-clap signal to indicate that a user is going to sleep or has woken up. A quick sound/light display follows recognition of the signal and then this data is stored in a SQL database using Flask such that users can easily access their sleep times for many days in the past if desired.

### sound/light/temperature tracking
As an added feature, the ESP32 also has connected a thermoresistor and photoresistor (in addition to the microphone) for recording sound/light/temperature levels (or more accurately disturbances) throughout the night. Data is again stored in a SQL database.
