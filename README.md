# ESP32ServoController
A PWM and servo library for ESP32 platform.

Please see the UML in the documentation page.

## Usage
Esp32LedcRegistry is a singleton used to register the hardware platform capabilities.
PWMController and ServoController are the main classes to be used.

## PWMController
Depending on the platform (ESP32 subtype) used, timers and channels (as used by both PWMController and ServoController) might be of type 'high speed' and/or 'low speed'.
Both PWMController and ServoController therefore require some guidance on which type should be used.
PWMController.begin requires a Esp32LedcFactory instance. Options for that are:
 - LowSpeedFactory: always creates timers and channels in the low speed variant, when available (read: not all used)
 - HighSpeedFactory: always creates timers and channels in the high speed variant, when available for the platform and not fully used
 - BestAvailableFactory: tries to create high speed timers/channels first. When that is (no longer) an option, it will try the low speed variants.
*BestAvailableFactory*, therefore is the easy option if you basically don't care.

## ServoController
For the ServoController.begin, a ServoFactoryDecorator is created.
This ServoFactoryDecorator wraps a Esp32LedcFactory, making the interface easier to use.
Servo specific settings (frequency, min. timing and max. timing) can all be globally set using Esp32LedcRegistry.

## Examples
There are two main examples:
 - PwmController, which demonstrates a basic PWM setup
 - ServoController, demonstrating a basic servo setup. Please read the comments near *setServoParams* carefully. By default the Esp32LedcRegistry will use 1 - 2 msec for 0 to 180 degrees.
 
In addition, there are two additional 'examples'. These are mainly used for my test purposes, 'test suite' is therefore a better description.
These 'examples' might provide some additional clarification, but these are not really meant for that..
 - PwmErrorHandlingTest_ESP32, is a test suite for a ESP32 setup (LilyGo T-Display)
 - PwmErrorHandlingTest_ESP32_S3, is a test suite for a ESP32-S3 (LilyGo T-Display-S3)
