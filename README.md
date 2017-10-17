# PWM-Frequency-Booster
Convert low pwm frequency of  ~980Hz up to 32Khz

If it's necessary to have higher frequency on puls width modulation than a standard MCU is able to and its not practicable to increase the controller timer frequency, you can use this solution.
If your fan works like a speaker, you know what I mean ;-)

An ATTiny85 or similar MCU, convert the low pwm-frequency of 976.5625Hz (e.g. a standard Arduino pwm frequency) up to 32Khz. You need only a cheap ATTiny model, this program and a 10k resistor. The duty cycle of the puls is not touched, that means:

  Input duty cycle = Output duty cycle

In this example an ATTiny85 is used.

  * The low pwm input is on pin3 (PB4), the high output pwm on pin5 (PB0).
  * On pin 1, a 10k is connected direct to VCC.

In the header section you can find some values to adjust the program according to your situation:

* PWM_INPUT       - PWM low frequency input pin
* PWM_OUTPUT	    - PWM high frequency output pin
* PWM_FREQ_HIGH	  - PWM frequency: 0 = 16KHz or 1 = 32KHz
* DUTY_MIN_LIMIT  - min/max limits according e.g. your fan
* DUTY_MAX_LIMIT	

Its important to erase the Fuse Low Byte CKDIV8 of the MCU to get 8MHz Clock frequency!
