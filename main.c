/*******************************************************************
*******	        PWMBooster V1.01 2017/10/23     ********************
******* by Wolfgang S. (eleschrott @github.com) ********************
********************************************************************/

#include <stdint.h>
#include <stdbool.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#define PWM_INPUT		PB4		// PWM low frequency input pin
#define PWM_OUTPUT		PB0		// PWM high frequency output pin

#define PWM_FREQ_HIGH		1		// PWM frequency: 0 = 16KHz
						//		  1 = 32KHz 

#define DUTY_MIN_LIMIT		2		// min/max limits according...
#define DUTY_MAX_LIMIT		245		// ... your fan

#define SAMPLE_RATE     	512		// 384, 1024, 1536... samples of input pulses to calculate the duty rate
#define SAMPLE_TIME		393216		// max. sample time to reset timer1 overflow, in dependence on sample rate!

volatile uint32_t		pulseOn = 0,
				pulseTime = 0,
				pulseLength = 0,
				pwmValue = 0,
				samples = 0;

volatile uint8_t		pwmDuty = 0,
				timerVal = 0;

volatile bool			fallingEdge = false,
				inputPulse = false,
				permaHigh = false;

bool				pwmOn = true;
			
int main()
{
	DDRB |= _BV(PWM_OUTPUT);	// set PWM_PIN as output
	PORTB &= ~_BV(PWM_OUTPUT);	// set PWM_PIN to LOW

	PCMSK |= _BV(PWM_INPUT);	// turn on interrupts on pin PWM_INPUT
	GIMSK |= _BV(PCIE);		// turns on pin change interrupts

	TCCR1 |= _BV(CS01);		// timer1 prescaler 8 = 1MHz 
	TIMSK |= _BV(TOIE1);		// set overflow interrupt on timer1

	sei();				// enable interrupts

	#if defined(PWM_FREQ_HIGH) && PWM_FREQ_HIGH == 1 // if PWM_FREQ_HIGH set to 1, use fast pwm mode
	TCCR0A |= _BV(COM0A1) | _BV(WGM01) | _BV(WGM00); // set OC0A on compare match and fast pwm mode with 32KHz
	#else
	TCCR0A |= _BV(COM0A1) | _BV(WGM00);		 // set OC0A on compare match and normal pwm with 16KHz
	#endif

	TCCR0B |= _BV(CS00);				 // timer0 no prescaling = 8MHz

	while (1)
	{
		if ((PINB & _BV(PWM_INPUT)) && permaHigh && (pulseTime >= SAMPLE_TIME)) // if only one high pulse, full power is needed!
		{
			if (pwmOn == false)
			{
				cli();
				DDRB	|= _BV(PWM_OUTPUT);
				TCCR0B	|= _BV(CS00);
				pwmOn = true;
				pwmDuty = 255;
				OCR0A = 255;  // full duty cycle
				sei();
			}
		}
		else if ((pwmOn == false) && (pwmDuty > DUTY_MIN_LIMIT) && (inputPulse == true)) // if PWM is switched of, switch it on
		{
			cli();
			pwmOn = true;
			DDRB	|= _BV(PWM_OUTPUT);
			TCCR0B	|= _BV(CS00);
			sei();
		}
		else if (((pwmOn == true) && (pwmDuty <= DUTY_MIN_LIMIT)) || ((pwmOn == true) && (inputPulse == false))) // stop pwm
		{
			cli();
			TCCR0B	&= ~_BV(CS00);		// stop timer0 
			PORTB	&= ~_BV(PWM_OUTPUT);	// set PWM_OUTPUT to LOW
			DDRB	&= ~_BV(PWM_OUTPUT);	// erase PWM_PIN from output register
			pwmOn = false;
			pwmValue = 0;
			samples = 0;
			sei();
		}
		
	}
}

// ISR for timer1 overflow
ISR(TIMER1_OVF_vect)
{
	pulseTime += 256;

	if (pulseTime >= SAMPLE_TIME) // if long time no pulse, erase flag to switch off the pwm!
	{
		inputPulse = false;
	}
}

// ISR for pin state change
ISR(PCINT0_vect)
{
	pulseLength = pulseTime;
	timerVal = TCNT1;		// save timer1 register
	permaHigh = false;

	if (PINB & _BV(PWM_INPUT))	// rising edge of pulse
	{	
		permaHigh = true;

		if (!fallingEdge)	// begin of pulse
		{
			fallingEdge = true;
			pulseTime = 0;
			pulseLength = 0;
			pulseOn = 0;
			TCNT1 = 0;	// start timer1
			return;
		}
		else // next rising edge, end of period
		{
			pulseLength += (uint32_t)timerVal;
			pwmValue += ((((pulseOn * 100) / pulseLength) * 256) / 100); // calculate duty cycle of this pulse

			samples++;
				
			fallingEdge = false;

			if (samples >= SAMPLE_RATE) // enough samples, calculate the duty
			{
				pwmValue = (uint8_t)(pwmValue / samples);

				if (pwmValue < DUTY_MIN_LIMIT)
				{
					pwmDuty = 0;
				}
				else if (pwmValue >=  DUTY_MAX_LIMIT)
				{
					pwmDuty = 255;
				}
				else if (pwmValue >= DUTY_MIN_LIMIT)
				{
					pwmDuty = (uint8_t)pwmValue;
				}
				inputPulse = true;
				pwmValue = 0;
				samples = 0;
				OCR0A = pwmDuty; // set duty to register
			}
			return;
		}
	}
	else if (fallingEdge) // falling edge
	{
		pulseLength += (uint32_t)timerVal; // get pulse-on time
		pulseTime = pulseLength;
		pulseOn = pulseLength;
	}
}
