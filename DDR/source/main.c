/*	Author: Briana McGhee
 *  Partner(s) Name: 
 *	Lab Section:
 *	Assignment: Lab #9  Exercise #3
 *	Exercise Description: Using the ATmega1284’s built in PWM functionality, design a system where a short,
five-second melody, is played when a button is pressed. NOTE: The melody must be somewhat
complex (scaling from C to B is NOT complex).
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 *
 *	Demo Link: https://youtu.be/dD0Qzl6OV0Y
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include "timer.h"
#include "pwm.h"
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

enum AStates {AStart, AInit, AStatus} A_s; //keeps track of health
enum BStates {BStart, BOn, BRelease, BPlay} B_s; //plays the melody
enum CStates {CStart, CInit, CLight} C_s;

unsigned char BB, GB, RB; //input buttons
unsigned char health; 
unsigned char R1, R2, G1, G2, Y1, Y2, B1, B2;
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
                        health = 0x07;
                        break;
                case AStatus:
                        PORTC = health;
                        break;
                default:
                        break;
        }
}

void Tick_B(){
	RB = ~PINA & 0x04;

	switch (B_s){
		case BStart:
			B_s = BOn;
			break;
		case BOn:
			if (RB){
				B_s = BRelease;
			} else {
				B_s = BOn;
			}
			break;
		case BRelease:
			B_s = BPlay;
			break;
		case BPlay:
			if (count < 8){
				B_s = BPlay;
			} else {
				B_s = BOn;
			}
			break;
		default:
			break;
	}

	switch (B_s){
		case BStart:
			break;
		case BOn:
			break;
		case BRelease:
			break;
		case BPlay:
			if (count < 8){
				count++;
				set_PWM(melody[count]);
			}
			break;
		default:
			break;
	}
}

void Tick_C(){
	R1 = PORTD & 0x80;
	G1 = PORTD & 0x40;
	Y1 = PORTD & 0x20;
	B1 = PORTD & 0x10;
	R2 = PORTD & 0x08;
	G2 = PORTD & 0x04;
	Y2 = PORTD & 0x02;
	B2 = PORTD & 0x01;

	switch(C_s){
		case CStart:
			C_s = CInit;
			break;
		case CInit:
			C_s = CLight;
			break;
		case CLight:
			C_s = CLight;
			break;
		default:
			break;
	}
	
	switch(C_s){
                case CStart:
                        break;
                case CInit:
			PORTD = 0x00;
                        break;
                case CLight:
			/*
			if (count == 0 || count == 4){
				R1 = 0x01;
				R2 = 0x01;
			} else if (count == 1 || count == 5){
				G1 = 0x01;
				G2 = 0x01;
			} else if (count == 2 || count == 6 ){
                                Y1 = 0x01;
                                Y2 = 0x01;
                        } else if (count == 3 || count == 7){
                                B1 = 0x01;
                                B2 = 0x01;
                        }
			*/
			PORTD = 0xFF;

                        break;
                default:
                        break;
        }
}

int main(void) {
    /* Insert DDR and PORT initializations */
	DDRA = 0x00; PORTA = 0xFF;
	DDRB = 0xFF; PORTB = 0x00;
	DDRC = 0xFF; PORTC = 0x00;
	DDRD = 0xFF; PORTD = 0x00;
    /* Insert your solution below */
	PWM_on();
	set_PWM(0);
	TimerSet(500);
	TimerOn();

    	while (1) {
		Tick_A();
		Tick_B();
		Tick_C();

		while(!TimerFlag){}
		TimerFlag = 0;
    	}
    return 1;
}
