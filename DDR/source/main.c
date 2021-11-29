/*	Author: Briana McGhee, bmcgh001@ucr.edu
 *  Partner(s) Name: 
 *	Lab Section: 21
 *	Assignment: Final Project - DDR (Demo 1)
 *	Exercise Description: Dance Dance Revolution is a music video game created by Konami in 1998. In this recreation, DDR is reduced to a mini-game embedded in C. The purpose of the game is to play through
tunes that have corresponding symbols appearing on the screen. The player controls a joystick
with a series of buttons and attempts to match the inputs to the symbols displayed on the
screen. Initially, the player is given 3 life points. If the player fails to match an input correctly, a
single life point is taken away. When all life points are gone, the game is over and the best
attempted score is recorded. If the player successfully completes a tune, the life points are
restored and they move on to the next level. The game is won when every tune has been
completed
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 *
 *	Complexity 1: https://youtu.be/GmyphbOv_po
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include "timer.h"
#include "pwm.h"
#include "adc.h"
#include <stdio.h>
#include <stdbool.h>

#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

enum AStates {AStart, AInit, AStatus, AReset} A_s; //keeps track of health
enum BStates {BStart, BOn, BPlay, BReset} B_s; //plays the melody
enum CStates {CStart, CInit, CLight, CReset} C_s; //plays the lights with the melody
enum DStates {DStart, DOff, firstNote, silence, DOn} D_s; //turns game on and off
enum EStates {EStart, EInit, EPlay, EWin, EReset} E_s; //play game
enum FStates {FStart, FInit, FStatus} F_s; //monitor joystick inputs

unsigned char BB, GB, YB, RB; //input buttons
unsigned char health[5] = {0x00, 0x00, 0x04, 0x06, 0x07}; int hCount = 0; //display and track health
unsigned char score; //track score
unsigned char count = 0; //cycle through melody and lights
unsigned short x, y; //joystick vertical and horizontal
unsigned char input; //result of joystick input

bool gameOn; //is the game on or off?
int songStatus; //0 = off 1 = on 2 = end;

unsigned char patterns[10] = {0x00, 0x80, 0x40, 0x20, 0x10, 0x0F, 0xF0, 0x0F, 0x00, 0xFF}; //lights

double melody[9] = {293.66, 261.63, 246.64, 220.00, 392.00, 329.63, 369.99, 329.63, 293.66}; //pallet town
//double melodyB[];
//double melodyC[];
//double melodyD[];
//double melodyE[];

int size = sizeof melody/ sizeof melody[0]; //size of the song

void reset(){
	A_s = AReset;
	B_s = AReset;
	C_s = AReset;
	E_s = EReset;
	PORTD = 0xFF;
}

int Tick_A(int state){ //keeps track of health
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
		case AReset:
			A_s = AStart;
                default:
                        break;
        }

        switch(A_s){
                case AStart:
                        break;
                case AInit:
                        hCount = 4;
                        break;
                case AStatus:
			PORTC = health[hCount];
		       	break;
		case AReset:
			break;
		default:
                        break;
        }
	return A_s;
}

int Tick_B(int state){ //plays the melody
	RB = ~PINA & 0x04;

	switch (B_s){
		case BStart:
			B_s = BOn;
			break;
		case BOn:
			if (RB && (gameOn == true)){
				B_s = BPlay;
			} else {
				B_s = BOn;
			}
			break;
		case BPlay:
			if (count <= size){
				B_s = BPlay;
			} else {
				B_s = BOn;
			}
			break;
		case BReset:
			B_s = BStart;
			break;
		default:
			break;
	}

	switch (B_s){
		case BStart:
			break;
		case BOn:
			songStatus = 0;
			count = 0;
			set_PWM(0);
			break;
		case BPlay:
			songStatus = 1;
			if (count <= size){
				set_PWM(melody[count]);
				count++;
			}

			if (count ==  size + 1){
				songStatus = 2;
			}
			break;
		case BReset:
			set_PWM(261.63);
			break;
		default:
			break;
	}
	return B_s;
}

int Tick_C(int state){ //plays the lights with the melody
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
			if (count > size){
				C_s = CInit;
			} else {
				C_s = CLight;
			}
			break;
		case CReset:
			C_s = CStart;
		default:
			break;
	}
	
	switch(C_s){
                case CStart:
                        break;
                case CInit:
                        break;
                case CLight:
			if (count > 0){
			  	PORTD = patterns[count];
			} else {
				PORTD = 0xFF;
			}
                        break;
		case CReset:
			break;
                default:
                        break;
        }
	return C_s;
}

int Tick_D(int state){ //turns game on and off
	switch(D_s){
		case DStart:
			D_s = DOff;
			break;
		case DOff:
			if (RB && (gameOn == false)) {
				D_s = firstNote;
				PORTD = 0xFF;
			} else {
				D_s = DOff;
			}
			break;
		case firstNote:
			D_s = silence;
			break;
		case silence:
			D_s = DOn;
			break;
		case DOn:
			if (RB && (gameOn == true) && (songStatus == 0)){
				D_s = DOff;
				gameOn = false;
			}
			break;
		default:
			break;
	}

	switch(D_s){
                case DStart:
                        break;
                case DOff:
			PORTD = 0x00;
                        break;
		case firstNote:
			set_PWM(440.00);
			break;
		case silence:
			set_PWM(0);
			gameOn = true;
			break;
                case DOn:
                        break;
                default:
                        break;
        }
	return D_s;
}

int Tick_E(int state){ //play game
	BB = ~PINA & 0x01;
	GB = ~PINA & 0x02;
	YB = ~PINA & 0x04;

	switch(E_s){
		case EStart:
			E_s = EInit;
			break;
		case EInit:
			E_s = EPlay;
			break;
		case EPlay:
			if (hCount == 0){
                                reset();
                        } else if (songStatus == 2 && hCount != 0){
				E_s = EWin;
			} else	{
                                E_s = EPlay;
                        }
			break;
		case EWin:
			break;
		case EReset:
			E_s = EStart;
			break;
		default:
			break;
	}

	switch(E_s){
                case EStart:
                        break;
                case EInit:
                        break;
                case EPlay:
			if (count % 2 == 0 && songStatus == 1){
				if (BB){
					if (hCount > 0){ 
						hCount--;
					}
				}
			} else if (count % 2 != 0 && songStatus == 1){
				if (GB){
					if (hCount > 0){
						hCount--;
					}
				}
			}
                        break;
		case EWin:
			score++;
			set_PWM(523.25);
			reset();
			break;
		case EReset:
			set_PWM(0);
			break;
                default:
                        break;
        }
	return E_s;
}

int Tick_F(int state){ //monitor joystick inputs
	x = ADC_read(5);
        y = ADC_read(4);

	switch(F_s){
		case FStart:
			F_s = FInit;
			break;
		case FInit:
			F_s = FStatus;
			break;
		case FStatus:
			F_s = FStatus;
			break;
		default:
			break;
	}
	
	switch(F_s){
                case FStart:
                        break;
                case FInit:
                        break;
                case FStatus:
			if (x >= MAX){
                		input = 1; //D
        		} else if (y >= MAX){
                		input = 2; //R
        		} else if (x <= MIN){
                		input = 3; //U
        		} else if (y <= MIN){
                		input = 4; //L
        		}
                        break;
                default:
                        break;
        }
	return F_s;
}
int main(void) {
    /* Insert DDR and PORT initializations */
	DDRA = 0x00; PORTA = 0xFF;
	DDRB = 0xFF; PORTB = 0x00;
	DDRC = 0xFF; PORTC = 0x00;
	DDRD = 0xFF; PORTD = 0x00;
    /* Insert your solution below */
	
	unsigned char i = 0;

	//keeps track of health
	tasks[i].state = AStart;
	tasks[i].period = 300;
	tasks[i].elapsedTime = 0;
	tasks[i].TickFct = &Tick_A;
	i++;

	//plays the melody
	tasks[i].state = BStart;
	tasks[i].period = 700;
	tasks[i].elapsedTime = 700;
	tasks[i].TickFct = &Tick_B;
	i++;

	//plays the lights with the melody
	tasks[i].state = CStart;
        tasks[i].period = 700;
        tasks[i].elapsedTime = 700;
        tasks[i].TickFct = &Tick_C;
	i++;

	//turns game on and off
	tasks[i].state = DStart;
	tasks[i].period = 500;
	tasks[i].elapsedTime = 0;
	tasks[i].TickFct = &Tick_D;
	i++;

	//play game
	tasks[i].state = EStart;
        tasks[i].period = 500;
        tasks[i].elapsedTime = 0;
        tasks[i].TickFct = &Tick_E;
	i++;

	//monitor joystick inputs
	tasks[i].state = FStart;
        tasks[i].period = 500;
        tasks[i].elapsedTime = 0;
        tasks[i].TickFct = &Tick_F;


        TimerSet(100);
        TimerOn();
	PWM_on();
	ADC_init();

    	while (1){
		
	/*	Used to test the joystick inputs

	 	x = ADC_read(5);
        	y = ADC_read(4);

		if(x >= MAX || y >= MAX){
                	PORTC = 0x01;
                } else if (x <= MIN || y <= MIN){
                        PORTC = 0x02;
                } else {
                	PORTC = 0x00;
                }                                       */

	
	}

        return 1;
}
