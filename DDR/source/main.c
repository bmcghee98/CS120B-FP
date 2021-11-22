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
#include <stdio.h>
#include <stdbool.h>

#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

enum AStates {AStart, AInit, AStatus} A_s; //keeps track of health
enum BStates {BStart, BOn, BRelease, BPlay} B_s; //plays the melody
enum CStates {CStart, CInit, CLight} C_s; //plays the lights with the melody
enum DStates {DStart, DOff, DOn} D_s; //turns game on and off

unsigned char BB, GB, RB; //input buttons
unsigned char health; 
bool gameOn, songOn;

unsigned char count = 0;

double melody[8] = {415.305, 349.23, 329.63, 493.88, 293.66, 415.305, 523.25, 261.63};


void Tick_A(){ //keeps track of health
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

void Tick_B(){ //plays the melody
	RB = ~PINA & 0x04;

	switch (B_s){
		case BStart:
			B_s = BOn;
			break;
		case BOn:
			songOn = false;
			if (RB && (gameOn == true)){
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
			songOn = true;
			if (count < 8){
				set_PWM(melody[count]);
				count++;
			}
			break;
		default:
			break;
	}
}

void Tick_C(){ //plays the lights with the melody
	switch(C_s){
		case CStart:
			C_s = CInit;
			break;
		case CInit:
			if (RB){
				C_s = CLight;
			} else {
				C_s = CInit;
			}
			break;
		case CLight:
			if (count > 8){
				C_s = CInit;
			} else {
				C_s = CLight;
			}
			break;
		default:
			break;
	}
	
	switch(C_s){
                case CStart:
                        break;
                case CInit:
			PORTD = 0xFF;
			set_PWM(0);
                        break;
                case CLight:
			
			if (count == 1 || count == 5){
				PORTD = 0x88;
			} else if (count == 2 || count == 6){
				PORTD = 0x44;
			} else if (count == 3 || count == 7){
                                PORTD = 0x22;
                        } else if (count == 4 || count == 8){
                                PORTD = 0x11;
                        }
			
                        break;
                default:
                        break;
        }
}

void Tick_D(){ //turns game on and off
	switch(D_s){
		case DStart:
			D_s = DOff;
			break;
		case DOff:
			if (RB && (gameOn == false)) {
				D_s = DOn;
				gameOn = true;
			} else {
				D_s = DOff;
			}
			break;
		case DOn:
			if (RB && (gameOn == true) && (songOn == false)){
				D_s = DOff;
				gameOn = false;
			} else {
				DOn;
			}
			break;
		default:
			break;
	}

	switch(D_s){
                case DStart:
                        break;
                case DOff:
                        break;
                case DOn:
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
		Tick_D();

		while(!TimerFlag){}
		TimerFlag = 0;
    	}
    return 1;
}
