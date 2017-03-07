/*

 * Created: 1/20/2017 1:41:23 PM
 * Author : Roberto Almeida;  ralme002@ucr.edu; SID: 861273226
 * Partner(s) Name & E-mail: Devin Naef; dnaef001@ucr.edu; SID:861305453
 
 * Lab Section: 022
 *  Assignment: Lab 9 Exercise 2 CS120B 001   
 * Exercise Description:  
    
 * Using the ATmega1284’s PWM functionality, design a system where the notes: C4, D, E, F, G, A, B, and C5,
   from the table at the top of the lab, can be generated on the speaker by scaling up or down the eight note scale.
    Three buttons are used to control the system. One button toggles sound on/off. The other two buttons scale up,
     or down, the eight note scale. r
 
 * I acknowledge all content contained herein, excluding template or example code, is my own original work.
*/

//Low throttle/arming is usually 1 to 1.1msec
//Full throttle is usually 1.9 to 2msec
//Frame rate is about 20msec (50hz)

//3 seconds of 1msec pulses for arming is about right.


#include <avr/io.h>
//ENUMS
enum Freq_State{START, WAIT,INC, DEC, INCRE, DECRE}freqstate;
enum ToneState{tSTART,PLAY} tonestate;
//END ENUMS

unsigned char A0,A1,A2=0;
double toneArray[]={500,478,477,476,475,474,473,470,460,440,430,420,410,400,390,380,370,360,350,340,330,320,310,300,290,280,270,250,511, 40,20,0}; //{250,500, 0}
unsigned char count =0;

unsigned char SetBit(unsigned char x, unsigned char k, unsigned char b)
{ return (b ? x | (0x01 << k) : x & ~(0x01 << k));}
unsigned char GetBit(unsigned char x, unsigned char k)
{ return ((x & (0x01 << k)) != 0);}


void set_PWM(double frequency) {
    
    
    // Keeps track of the currently set frequency
    // Will only update the registers when the frequency
    // changes, plays music uninterrupted.
    static double current_frequency;
    if (frequency != current_frequency) {

        if (!frequency) TCCR3B &= 0x08; //stops timer/counter
        else TCCR3B |= 0x03; // resumes/continues timer/counter
        
        // prevents OCR3A from overflowing, using prescaler 64
        // 0.954 is smallest frequency that will not result in overflow
        if (frequency < 0.954) OCR3A = 0xFFFF;
        
        // prevents OCR3A from underflowing, using prescaler 64					// 31250 is largest frequency that will not result in underflow
        else if (frequency > 31250) OCR3A = 0x0000;
        
        // set OCR3A based on desired frequency
        else OCR3A = (short)(8000000 / (128 * frequency)) - 1;

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

char FreqSM()
{
    switch(freqstate)
    {
        case START:
        PORTC =0x01;

        freqstate = WAIT;
        break;
        case WAIT:
        PORTC =0x02;
        if(A0)
        {
            freqstate = INC;
        }
        if(A1)
        {
            freqstate = DEC;
        }
        break;
        case INC:
        PORTC =0x04;
        if(A0)
        {
            freqstate =INCRE;
            if(count <27)
            {count++;}
            if(A1)
            {
                count =0;
            }
        }
        else
        {
            freqstate = WAIT;
        }
        break;
        case DEC:
        if(A1)
        {
            freqstate =DECRE;
            if(count>0)
            {count--;}
            if(A0)
            {
                count=0;
            }
        }
        else
        {
            freqstate = WAIT;
        }
        break;
        case INCRE:
        if(A0)
        {
            freqstate = INCRE;
        }
        else
        {
            freqstate = WAIT;
        }
        break;
        case DECRE:
        if(A1)
        {
            freqstate = DECRE;
        }
        else
        {
            freqstate = WAIT;
        }
        break;

    }
    PORTC = count;
    return 0;
}

char ToneSM()
{
    switch(tonestate)
    {
       case tSTART:
                   tonestate = PLAY;
                   break;
       case PLAY:
                 if(A2)
                 {
                  set_PWM(toneArray[count]);
                 }
                 else
                 {
                 set_PWM(0);
                 }
                 break;
        default: 
                tonestate =tSTART;
    }

return 0;
}

int main(void)
{

    DDRA = 0x00; PORTA = 0xFF;
    DDRB = 0xFF; PORTB = 0x00;
    DDRC = 0xFF; PORTC = 0x00;
    PWM_on();

    while (1) 
    {
    A0 = !GetBit(PINA,0);
    A1 = !GetBit(PINA,1);
    A2 = GetBit(PINA,2);
    
    FreqSM();
    ToneSM();
    PORTC=count;
 
    }
}

