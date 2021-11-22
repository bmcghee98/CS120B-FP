/*	Author: Briana McGhee, bmcgh001@ucr.edu
 *  Partner(s) Name: 
 *	Lab Section: 21
 *	Assignment: Final Project - DDR (Demo 1)
 *	Exercise Description: 
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 *
 *	Demo Link:
 */

#include <avr/io.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#include "pwm.h"
#include "timer.h"
#endif

unsigned char BB, GB, RB, health;

enum AStates {AStart, AInit, AStatus} A_s; //keeps track of health
enum States {Start, ON, RELEASE, ONE, TWO, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT} state;
unsigned char A0;
unsigned char count = 0;
double melody[8] = {415.305, 349.23, 329.63, 493.88, 293.66, 415.305, 523.25, 261.63};


void Tick_A(){
        switch(A_s){
                case AStart:
                        A_s = AInit;
                        break;
                case AInit:
                        A_s = AStatus;
                        break;
                case AStatus:
                        A_s = AStatus;
                        break;
                default:
                        break;
        }

        switch(A_s){
                case AStart:
                        break;
                case AInit:
                        health = 0x02;
                        break;
                case AStatus:
                        PORTC = health;
                        break;
                default:
                        break;
        }
}

void Tick(){
	unsigned char A0 = ~PINA & 0x01;

	switch (state){
		case Start:
			state = ON;
			break;
		case ON:
			if (A0){
				state = RELEASE;
			} else {
				state = ON;
			}
			break;
		case RELEASE:
			state = ONE;
			break;
		case ONE:
			state = TWO;
			break;
		case TWO:
			state = THREE;
			break;
		case THREE:
			state = FOUR;
			break;
		case FOUR:
			state = FIVE;
			break;
		case FIVE:
			state = SIX;
			break;
		case SIX:
			state = SEVEN;
			break;
		case SEVEN:
			state = EIGHT;
			break;
		case EIGHT:
			state = ON;
			break;
		default:
			break;
	}

	switch (state){
		case Start:
			break;
		case ON:
			break;
		case RELEASE:
			break;
		case ONE:
			set_PWM(melody[0]);
			break;
		case TWO:
			set_PWM(melody[1]);
			break;
		case THREE:
			set_PWM(melody[3]);
			break;
		case FOUR:
			set_PWM(melody[4]);
			break;
		case FIVE:
			set_PWM(melody[5]);
			break;
		case SIX:
			set_PWM(melody[6]);
			break;
		case SEVEN:
			set_PWM(melody[7]);
			break;
		case EIGHT:
			set_PWM(melody[8]);
		default:
			break;
	}
}


int main(void) {
	DDRA = 0x00; PORTA = 0xFF;
	DDRB = 0xFF; PORTB = 0x00;
	DDRD = 0xFF; PORTD = 0x00;
	DDRC = 0xFF; PORTC = 0x00;
	
	DDRB = 0xFF; PORTB = 0x00;
    /* Insert your solution below */
	PWM_on();
	set_PWM(0);
	TimerSet(700);
	TimerOn();

    while (1) {
	Tick();
	Tick_A();

	while(!TimerFlag){}
	TimerFlag = 0;
    }
	
    	return 1;
}
