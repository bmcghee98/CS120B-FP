#ifndef __ADC_H__
#define __ADC_H__

#define MAX 600
#define MIN 100
void ADC_init(){
	ADMUX |= (1 << REFS0);
	ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}

/*
 * Resources:
 * https://circuitdigest.com/microcontroller-projects/how-to-use-adc-in-avr-microcontroller-atmega16
 * https://maxembedded.com/2011/06/the-adc-of-the-avr/
 *
 * Is this okay? 
*/

unsigned int ADC_read(unsigned char pin)
{
	pin = pin & 0b00000111;
	ADMUX = (ADMUX & 0xF8) | pin;
	ADCSRA |= (1 << ADSC);

	while(ADCSRA & (1 << ADSC));

	return ADC;
}

#endif
