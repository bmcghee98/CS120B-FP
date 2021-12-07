/*	Author: Briana McGhee, bmcgh001@ucr.edu
 *  Partner(s) Name: 
 *	Lab Section: 21
 *	Assignment: Final Project - DDR (Final Demo)
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
 *	Complexity 2: https://youtu.be/roRe8nDgkyE
 *	Complexity 3: https://youtu.be/VkaSIxvdEtM
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include "io.h"
#include "timer.h"
#include "pwm.h"
#include "adc.h"
#include <stdio.h>
#include <stdbool.h>

#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

enum AStates {AStart, AInit, AStatus, AReset} A_s; //keeps track of health
enum BStates {BStart, BInit, BOn, BPlay, BReset} B_s; //plays the melody
enum CStates {CStart, CInit, CLight, CReset} C_s; //plays the lights with the melody
enum DStates {DStart, DOff, firstNote, silence, DOn} D_s; //turns game on and off
enum EStates {EStart, EInit, OPTION1A, OPTION1B, OPTION1C, HSCORE, EPlay, EWin, ELose, EReset} E_s; //the actual game
enum FStates {FStart, FInit, FStatus} F_s; //monitor joystick inputs
//enum GStates {GStart, GInit, GPlay, GEnd} G_s; //LCD screen

unsigned char BB, GB, YB, RB; //buttons
unsigned short x, y; //joystick vertical and horizontal
unsigned char input; //result of joystick input

unsigned char health[5] = {0x00, 0x00, 0x04, 0x06, 0x07}; int hCount = 0; //display and track health
unsigned char score, eScore; //track score, save to EEPROM

unsigned char count = 0; //cycle through melody and lights
unsigned char column = 32; //moves LCD cursor with joystick
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
	PORTD = 0x3F;
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
			PORTB = health[hCount];
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
		case BInit:
			B_s = BOn;
			break;
		case BOn:
			if (songStatus == 1 && (gameOn == true)){
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
		case BInit:
			songStatus = 0;
		case BOn:
			count = 0;
			set_PWM(0);
			break;
		case BPlay:
			if (count <= size){
				set_PWM(melody[count]);
				count++;
			}

			if (count >=  size + 1){
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
			if (songStatus == 1){
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
				PORTD = 0x3F;
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
				PORTD = 0x3F;
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
			eScore = eeprom_read_byte(0);
			PORTD = 0x00;
                        break;
		case firstNote:
			set_PWM(440.00);
			break;
		case silence:
			set_PWM(0);
                        LCD_DisplayString(1, "Welcome to DDR! START * ");
			LCD_WriteData(eScore);
			gameOn = true;
			break;
                case DOn:
                        break;
                default:
                        break;
        }
	return D_s;
}

int Tick_E(int state){ //the actual game
	BB = ~PINA & 0x01;
	GB = ~PINA & 0x02;
	YB = ~PINA & 0x08;

	eScore = 0;

	switch(E_s){
		case EStart:
			E_s = EInit;
			break;
		case EInit:
			if (((column - 17) * (column - 23) < 0) && YB){
				E_s = OPTION1A;
			} else if (column == 24 && YB){
				E_s = HSCORE;
			}
			break;
		case OPTION1A:
			E_s = OPTION1B;
			break;
		case OPTION1B:
			E_s = OPTION1C;
			break;
		case OPTION1C:
			E_s = EPlay;
			break;
		case HSCORE:
			if (YB){
				E_s = EInit;
			}
			break;
		case EPlay:
			if (hCount == 0){
				E_s = ELose;
                        } else if (songStatus == 2 && hCount != 0){
				E_s = EWin;
			} else	{
                                E_s = EPlay;
                        }
			break;
		case EWin:
			break;
		case ELose:
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
		case OPTION1A:
			LCD_DisplayString(1, "Get ready...");
                        break;
		case OPTION1B:
			LCD_DisplayString(1, "Set...");
			break;
		case OPTION1C:
			LCD_DisplayString(1, "GO!");
			songStatus = 1;
			break;
                case HSCORE:
			LCD_WriteData(eScore + '0');
                        break;
                case EPlay:
			if (count % 2 == 0 && count != 9 && songStatus == 1){
				LCD_DisplayString(1, "           Green");
				if (BB || YB){
					if (hCount > 0){ 
						hCount--;
					}
				}
			} else if (count != 5 && count != 1 && count % 2 != 0 && songStatus == 1){
				LCD_DisplayString(1, "    Blue");
				if (GB || YB){
					if (hCount > 0){
						hCount--;
					}
				}
			} else if (count == 5 || count == 1 || count == 9){
				LCD_DisplayString(1, "Yellow");
				if (BB || GB){
					if (hCount > 0){
						hCount--;
					}
				}
			}
                        break;
		case EWin:
			LCD_DisplayString(1, "You won! +1     Continue? Y/N");
			score++;

			if (score == 1){
				eeprom_write_byte(0, score);
			} else if (score > 1) {
				eScore = eeprom_read_byte(0);

				if (eScore < score){
					eeprom_write_byte(0, score);
					//FIX
				}
			}
			set_PWM(523.25);
			reset();
			break;
		case ELose:
			eScore = eeprom_read_byte(0);
			if (eScore < score){
				eeprom_write_byte(0, score);
				LCD_DisplayString(1, "You lose!       Score saved");
			} else {
				LCD_DisplayString(1, "You lose!");
				score = 0;
			}
			eeprom_write_byte(0, score);
			break;
		case EReset:
			reset();
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
			if (x > MAX){
                		input = 1; //R
				if (column <= 31){
					column++;
				}
        		} else if (y > MAX){
                		input = 2; //D
				if (column < 17){
					column += 16;
				}
        		} else if (x < MIN){
                		input = 3; //L
				if (column > 1){
					column--;
				}
        		} else if (y < MIN){
                		input = 4; //U
				if (column > 16){
					column -= 16;
				}
        		}
			LCD_Cursor(column);
                        break;
                default:
                        break;
        }
	return F_s;
}

/*
//LCD Screen
int Tick_G(int state){
	switch(G_s){
		case GStart:
			G_s = GInit;
			break;
		case GInit:
			if (songStatus == 1){
				G_s = GPlay;
			}
			break;
		case GPlay:
		       if (songStatus == 0){
				G_s = GInit;
		       } else if (songStatus == 2){
				G_s = GEnd;
			} else {
				G_s = GPlay;
			}
			break;
		case GEnd:
			if (hCount != 0){
				LCD_DisplayString(1, "You won! +1 score");
			} else if (hCount <= 0){
				LCD_DisplayString(1, "You lost! Game over");
			}
			break;
		default:
			break;
	}

	switch(G_s){
                case GStart:
                        break;
                case GInit:
                        break;
                case GPlay:
			
                        break;
		case GEnd:
			break;
                default:
                        break;
        }
}

*/

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
	tasks[i].period = 800;
	tasks[i].elapsedTime = 800;
	tasks[i].TickFct = &Tick_B;
	i++;

	//plays the lights with the melody
	tasks[i].state = CStart;
        tasks[i].period = 800;
        tasks[i].elapsedTime = 800;
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
        tasks[i].period = 100;
        tasks[i].elapsedTime = 0;
        tasks[i].TickFct = &Tick_F;

        TimerSet(100);
        TimerOn();
	PWM_on();
	ADC_init();
	LCD_init();
	
    	while (1){
		
/*		//Used to test the joystick inputs

	 	x = ADC_read(5);
        	y = ADC_read(4);

		if(x >= MAX || y >= MAX){
                	PORTB = 0x01;
                } else if (x <= MIN || y <= MIN){
                        PORTB = 0x02;
                } else {
                	PORTB = 0x00;
                }                                       
*/	
	}

        return 1;
}
