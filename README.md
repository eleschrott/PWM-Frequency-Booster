# PWM-Frequency-Booster
Convert low pwm frequency of  ~980Hz up to 32Khz

If it's necessary to have higher frequency on puls width modulation (pwm) than a standard MCU is able to and its not practicable to increase the MCU timer frequency, you can use this solution. If you fan works like a speaker, you know what I mean ;-)

An ATTiny85 or similar MCU, convert the low frequency of 976.5625Hz (standard Arduino pwm frequency) up to 32Khz. You need only a cheap ATTiny model with this program and a 10K resistor.
