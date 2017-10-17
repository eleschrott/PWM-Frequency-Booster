#define F_CPU 8000000UL

#include <stdbool.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#define PWM_INPUT	PB4	// PWM low frequency input pin
#define PWM_OUTPUT	PB0	// PWM high frequency output pin

#define PWM_FREQ_HIGH	1	// PWM frequency:	0 = 16KHz
				//			1 = 32KHz 

#define DUTY_MIN_LIMIT	10	// min/max limits according...
#define DUTY_MAX_LIMIT	250	// ... your fan

#define SAMPLE_RATE     128	// samples of input pulses to calculate the duty rate
#define OUTIN_RATIO	32768	// output/input ratio to reset timer1 overflow

volatile uint32_t 	pulseOn,
			pulseLength,
			pwmValue;
volatile uint8_t	pwmDuty = 0,
			samples = 0;
volatile bool		first = false,
			pwmOn = true,
			inputPulse = false;
			
int main()
{
	DDRB |= _BV(PWM_OUTPUT);	// set PWM_PIN as output
	PORTB &= ~_BV(PWM_OUTPUT);	// set PWM_PIN to LOW

	PCMSK |= _BV(PWM_INPUT);	// turn on interrupts on pin PWM_INPUT
	GIMSK |= _BV(PCIE);		// turns on pin change interrupts

	TCCR1 |= _BV(CS01);		// timer1 prescaler 8 = 1MHz 
	TIMSK |= (1<<TOIE1);		// set overflow interrupt on timer1

	sei();				// enable interrupts

#if defined(PWM_FREQ_HIGH) && PWM_FREQ_HIGH == 1 	 // if PWM_FREQ_HIGH set to 1, use fast pwm mode
	TCCR0A |= _BV(COM0A1) | _BV(WGM01) | _BV(WGM00); // set OC0A on compare match and fast pwm mode with 32KHz
#else
	TCCR0A |= _BV(COM0A1) | _BV(WGM00);		 // set OC0A on compare match and normal pwm with 16KHz
#endif

	TCCR0B |= _BV(CS00);				 // timer0 no prescaling = 8MHz

	while (1)
	{
		if ((pwmDuty > DUTY_MIN_LIMIT) && (pwmOn == false) && (inputPulse == true)) // if PWM is switched of, switch it on
		{
			pwmOn = true;
			DDRB	|= _BV(PWM_OUTPUT);
			TCCR0B	|= _BV(CS00);
		}
		else if (((pwmDuty <= DUTY_MIN_LIMIT) && (pwmOn == true)) || (inputPulse == false)) // stop pwm
		{
			OCR0A = 0;
			pwmOn = false;
			TCCR0B	&= ~_BV(CS00);	     // stop timer0 
			PORTB	&= ~_BV(PWM_OUTPUT); // set PWM_OUTPUT to LOW
			DDRB	&= ~_BV(PWM_OUTPUT); // erase PWM_PIN from output register
			}

			if (pwmOn)
			{
				OCR0A = pwmDuty;
			}
		}
}

// ISR for timer1 overflow
ISR (TIMER1_OVF_vect)
{
	cli();
	
	pulseLength += 255; 

	if (pulseLength > OUTIN_RATIO) // if long time no pulse, erase flag to switch off the pwm!
	{
		inputPulse = false;
	}

	sei();
	return;
}

// ISR for pin state change
ISR(PCINT0_vect)
{
	uint8_t timerVal;
	
	cli();

	timerVal = TCNT1;  // save timer1 register

	if (samples >= SAMPLE_RATE) // enough samples, calculate the duty
	{
		pwmValue = (pwmValue / samples);

		if ((uint8_t)pwmValue < DUTY_MIN_LIMIT)
		{
			pwmDuty = 0;
		}
		else if ((uint8_t)pwmValue >=  DUTY_MAX_LIMIT)
		{
			pwmDuty = 255;
		}
		else if ((uint8_t)pwmValue >= DUTY_MIN_LIMIT)
		{
			pwmDuty = (uint8_t)pwmValue;
		}
		inputPulse = true;
		pwmValue = 0;
		samples = 0;
	}

	if (PINB & _BV(PWM_INPUT)) // rising edge of pulse
	{
		if (!first) // begin of pulse
		{
			first = true;
			pulseLength = 0;
			pulseOn = 0;

			TCNT1 = 0;  // start timer1

			sei();
			return;
		}
		else // next rising edge, end of period
		{
			pulseLength += (uint32_t)timerVal;
			pwmValue += ((((pulseOn * 100) / pulseLength) *255) / 100); // calculate duty cycle for this pulse an add it

			samples++;
			first = false;
			TCNT1 = 0;	// reset timer1

			sei();
			return;
		}
	}
	else if (first) // falling edge
	{
		pulseLength += (uint32_t)timerVal; // get pulse-on time 
		pulseOn = pulseLength;
	}
		
	sei();
	return;
}
