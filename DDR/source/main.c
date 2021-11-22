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
enum EStates {EStart, EInit, EPlay} E_s; //play game

unsigned char BB, GB, RB; //input buttons
unsigned char health[4] = {0x00, 0x01, 0x02, 0x07}; int hCount = 0;
bool gameOn, songOn;

unsigned char count = 0;

//double melody[8] = {415.305, 349.23, 329.63, 493.88, 293.66, 415.305, 523.25, 261.63};
double melody[9] = {293.66, 261.63, 246.64, 220.00, 392.00, 329.63, 369.99, 329.63, 293.66}; //pallet town

//int Tick_A(int state){ //keeps track of health
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
                        hCount = 3;
                        break;
                case AStatus:
                        PORTC = health[hCount];
                        break;
                default:
                        break;
        }
//	return A_s;
}

//int Tick_B(int state){ //plays the melody
void Tick_B(){
	RB = ~PINA & 0x04;
	int size = sizeof melody/ sizeof melody[0];

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
			if (count <= size){
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
			count = 0;
			set_PWM(0);
			break;
		case BRelease:
			break;
		case BPlay:
			songOn = true;
			if (count <= size){
				set_PWM(melody[count]);
				count++;
			}
			break;
		default:
			break;
	}
	//return B_s;
}

//int Tick_C(int state){ //plays the lights with the melody
void Tick_C(){
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
	//return C_s;
}

//int Tick_D(int state){ //turns game on and off
void Tick_D(){
	switch(D_s){
		case DStart:
			D_s = DOff;
			break;
		case DOff:
			if (RB && (gameOn == false)) {
				D_s = DOn;
				PORTD = 0xFF;
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
	//return D_s;
}

//int Tick_E(int state){
void Tick_E(){
	BB = ~PINA & 0x01;
	GB = ~PINA & 0x02;

	switch(E_s){
		case EStart:
			E_s = EInit;
			break;
		case EInit:
			E_s = EPlay;
			break;
		case EPlay:
			E_s = EPlay;
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
			if (count % 2 == 0){
				if (BB){
					if (hCount > 0){ 
						hCount--;
					}
				}
			} else if (count % 2 != 0){
				if (GB){
					if (hCount > 0){
						hCount--;
					}
				}
			}
                        break;
                default:
                        break;
        }
	//return E_s;
}

int main(void) {
    /* Insert DDR and PORT initializations */
	DDRA = 0x00; PORTA = 0xFF;
	DDRB = 0xFF; PORTB = 0x00;
	DDRC = 0xFF; PORTC = 0x00;
	DDRD = 0xFF; PORTD = 0x00;
    /* Insert your solution below */
	
	/*
	unsigned char i = 0;	
	tasks[i].state = AStart;
	tasks[i].period = 50;
	tasks[i].elapsedTime = 0;
	tasks[i].TickFct = &Tick_A;
	i++;
	tasks[i].state = BStart;
	tasks[i].period = 700;
	tasks[i].elapsedTime = 700;
	tasks[i].TickFct = &Tick_B;
	i++;
	tasks[i].state = CStart;
        tasks[i].period = 1000;
        tasks[i].elapsedTime = 0;
        tasks[i].TickFct = &Tick_C;
	i++;
	tasks[i].state = DStart;
	tasks[i].period = 500;
	tasks[i].elapsedTime = 0;
	tasks[i].TickFct = &Tick_D;
	i++;
	tasks[i].state = EStart;
        tasks[i].period = 100;
        tasks[i].elapsedTime = 0;
        tasks[i].TickFct = &Tick_E;

	*/
	PWM_on();
        set_PWM(0);
        TimerSet(500);
        TimerOn();

    	while (1) {
		Tick_A();
		Tick_B();
		Tick_C();
		Tick_D();
		Tick_E();
		
		while(!TimerFlag){}
		TimerFlag = 0;
	
	}
        return 1;
}
