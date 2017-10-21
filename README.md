# PWM-Frequency-Booster
### Convert low pwm frequency of  ~980Hz up to 32Khz

If it's necessary to have higher frequency on puls width modulation than a standard MCU is able to and its not practicable to increase the controller timer frequency, you can use this solution.
If your fan works like a speaker, you know what I mean :smirk:

An ATTiny85 or similar MCU, convert the low pwm-frequency of **976.5625Hz** (e.g. a standard Arduino pwm frequency) up to
**32KHz**. You need only a cheap ATTiny model, this program and a 10k resistor. The duty cycle of the puls is not touched, that means:
````
Input Duty Cycle = Output Duty Cycle
````
#### In this example an ATTiny85 is used. <p align="left"><img src="img/attinyx5.png" width="550"/></p>

---
  
<p align="left"><img src="img/schematic.png" width="550"/></p>

 * The low pwm input is on pin3 (PB4), the high output pwm on pin5 (PB0).
 * On pin 1, a 10k resistor is connected direct to VCC.
 * The two caps are standard	blocking capacitors
  
---
In the header section you can find some values to adjust the program according to your situation:
```c++
#define PWM_INPUT      // PWM low frequency input pin
#define PWM_OUTPUT     // PWM high frequency output pin
#define PWM_FREQ_HIGH  // PWM frequency: 0 = 16KHz or 1 = 32KHz
#define DUTY_MIN_LIMIT // min limit according your fan 
#define DUTY_MAX_LIMIT // max limit according your fan
```

One hint to program the MCU:

--> Its important to erase the **Fuse Low Byte CKDIV8** of the MCU to get 8MHz Clock frequency!
