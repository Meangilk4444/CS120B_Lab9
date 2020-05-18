/*	Author: lab
 *  Partner(s) Name: karen
 *	Lab Section:
 *	Assignment: Lab #  Exercise #
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */
#ifdef _SIMULATE_
#include <avr/io.h>
#include "simAVRHeader.h"
#include <avr/interrupt.h>

#endif

volatile unsigned char TimerFlag = 0;

unsigned long _avr_timer_M = 1;
unsigned long _avr_timer_cntcurr = 0;

void TimerOn(){
	TCCR1B = 0x0B;
	OCR1A = 125;
	TIMSK1 = 0x02;
	TCNT1 = 0;
	_avr_timer_cntcurr = _avr_timer_M;
	SREG |= 0x80;
}

void TimerOff(){
	TCCR1B = 0x00;
}

void TimerISR(){
	TimerFlag = 1;
}

ISR(TIMER1_COMPA_vect){
	_avr_timer_cntcurr--;
	if(_avr_timer_cntcurr ==0){
		TimerISR();
		_avr_timer_cntcurr = _avr_timer_M;
	}
}

void TimerSet(unsigned long M){
	_avr_timer_M = M;
	_avr_timer_cntcurr = _avr_timer_M;
}

void set_PWM(double frequency) {
	// Keeps track of the currently set frequency
	// Will only update the registers when the frequency
	// changes, plays music uninterrupted.
	static double current_frequency;
	if (frequency != current_frequency) {
		if (!frequency) {TCCR3B &= 0x08;} //stops timer/counter
		else {TCCR3B |= 0x03;} // resumes/continues timer/counter
		
		// prevents OCR3A from overflowing, using prescaler 64
		// 0.954 is smallest frequency that will not result in overflow
		if (frequency < 0.954){OCR3A = 0xFFFF;}
		
		// prevents OCR3A from underflowing, using prescaler 64			       // 31250 is largest frequency that will not result in underflow
		else if (frequency > 31250) {OCR3A = 0x0000;}
		
		// set OCR3A based on desired frequency
		else {OCR3A = (short)(8000000 / (128 * frequency)) - 1;}

		TCNT3 = 0; // resets counter
		current_frequency = frequency;
	}
}
void PWM_on() {
	TCCR3A = (1 << COM3A0);
	// COM3A0: Toggle PB6 on compare match between counter and OCR3A
	TCCR3B = (1 << WGM32) | (1 << CS31) | (1 << CS30);
	// WGM32: When counter (TCNT3) matches OCR3A, reset counter
	// CS31 & CS30: Set a prescaler of 64
	set_PWM(0);
}
void PWM_off() {
	TCCR3A = 0x00;
	TCCR3B = 0x00;
}

int count = 0x00;
double notes[8] = {261.63, 293.66, 349.23, 329.63, 349.23,523.25,  392.00, 440.00, 261.63, 493.88, 523.25};
unsigned char button = 0x00;
unsigned char on_off = 0x00;
enum States{START, INIT, MUSIC, SOUND, ADD, SUB, WAIT} state;
void Tick()
{
	button = ~PINA & 0x01;
	switch(state)
	{
		case START:
			state = INIT;
			break;

		case INIT:
			if(button) //button is pressed
			{
				state = MUSIC;
			}
			else
			{
				state = INIT;
			}
			break;

			case MUSIC:
			

		default:
			state = START;
			break;
	}

	switch(state)
	{
		case START:
			set_PWM(0);
			break;

		case MUSIC:
			break;

		case SOUND:
			if(on_off == 0x00) //turn on
			{
				PWM_on();
				on_off = 0x01;
			}
			else
			{
				PWM_off();
				on_off = 0x00;
			}
			break;

		case ADD:
			if(count < 7)
			{
				count++;
			}
			break;

		case SUB:
			if(count > 0)
			{
				count--;
			}
			break;

		case WAIT:
			set_PWM(notes[count]);
			break;
	}	


}

int main(void){
	DDRA = 0x00; PORTA = 0xFF;
	DDRB = 0xFF; PORTB = 0x00;
	state = START;
	PWM_on();

	while(1){
		Tick();
	}

return 0;

}
