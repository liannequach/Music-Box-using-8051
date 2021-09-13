#include <reg51.h>
#include <intrins.h> //for _nop_()

#define uchar unsigned char
#define uint unsigned int
	
#define C4 1+7
#define D4 2+7
#define E4 3+7
#define F4 4+7
#define G4 5+7
#define A4 6+7
#define B4 7+7
#define C5 1+14

//define the 4 model control pins
sbit song1 = P0^0;
sbit song2 = P0^1;
sbit song3 = P0^2;
sbit song4 = P0^3;

sbit Buzzer = P3^7;

bit chg_song; //change song flag
volatile uchar th0, tl0; //hold current tone information

struct music_type 
{ 
	uchar tone;
	uchar delay;
};

uchar th0, tl0; //hold the current timer initial value

//intial value table for C scale 3 groups, each has 7 tones

												// C  ,   D  ,   E  ,   F  ,   G  ,   A  ,   B   
uint code tonetab_C[] = {62018, 62401, 62743, 62903, 63185, 63441, 63670, //c3
												 63777, 63969, 64140, 64216, 64360, 64489, 64603, //c4
												 64655, 64751, 64837, 64876, 64948, 65012, 65070}; //c5

//note table for Mary Had a Little Lamb
struct music_type code maryhadalittlelamb[] = {
	E4, 0x04, D4, 0x04, C4, 0x04, D4, 0x04, E4, 0x04, 0, 1 , E4, 0x04, 0, 1, E4, 0x08,
  D4, 0x04, 0, 1 , D4, 0x04, 0, 1, D4, 0x08, E4, 0x04, G4, 0x04, 0, 1, G4, 0x08,
	E4, 0x04, D4, 0x04, C4, 0x04, D4, 0x04, E4, 0x04, 0 , 1, E4, 0x04, 0, 1, E4, 0x08,
	D4, 0x04, 0, 1, D4, 0x04, E4, 0x04, D4, 0x04, C4, 0x012, 0 , 0
};

//note table for Twinkle Twinkle Little Star
struct music_type code twinkletwinkle[] = { 
	C4, 0x04, 0, 1, C4, 0x04, G4, 0x04, 0, 1, G4, 0x04, A4, 0x04, 0, 1, A4, 0x04, G4, 0x08, 
	F4, 0x04, 0, 1, F4, 0x04, E4, 0x04, 0, 1, E4, 0x04, D4, 0x04, 0, 1, D4, 0x04, C4, 0x08, 		
	G4, 0x04, 0, 1, G4, 0x04, F4, 0x04, 0, 1, F4, 0x04, E4, 0x04, 0, 1, E4, 0x04, D4, 0x08, 
	G4, 0x04, 0, 1, G4, 0x04, F4, 0x04, 0, 1, F4, 0x04, E4, 0x04, 0, 1, E4, 0x04, D4, 0x08, 		
	C4, 0x04, 0, 1, C4, 0x04, G4, 0x04, 0, 1, G4, 0x04, A4, 0x04, 0, 1, A4, 0x04, G4, 0x08, 
	F4, 0x04, 0, 1, F4, 0x04, E4, 0x04, 0, 1, E4, 0x04, D4, 0x04, 0, 1, D4, 0x04, C4, 0x08, 
	0, 0
};

//note table for Happy Birthday
struct music_type code happybirthday[] = {
	C4, 0x03, 0, 1, C4, 0x01, D4, 0x04, C4, 0x04, F4, 0x04, E4, 0x08, C4, 0x03, 0, 1,
	C4, 0x01, D4, 0x04, C4, 0x04, G4, 0x04, F4, 0x08, C4, 0x03, 0, 1, C4, 0x01,
	C5, 0x04, A4, 0x04, F4, 0x04, E4, 0x04, D4, 0x04, B4, 0x03, 0, 1, B4, 0x01,
	A4, 0x04, F4, 0x04, G4, 0x04, F4, 0x12, 0, 0
};

//own music
struct music_type code jinglebells[] = {
	E4, 0x02, 0, 1, E4, 0x02, 0, 1, E4, 0x04, 0, 1, E4, 0x02, 0, 1, E4, 0x02, 0, 1, E4, 0x04, 0, 1,
	E4, 0x02, G4, 0x02, C4, 0x02, D4, 0x02, E4, 0x08, 
	F4, 0x02, 0 , 1, F4, 0x02, 0, 1, F4, 0x04, 0, 1, F4, 0x02, E4, 0x02, 0, 1, E4, 0x04, 0, 1,
	E4, 0x02, D4, 0x02, 0, 1, D4, 0x02, E4, 0x02, D4, 0x04, G4, 0x04, 0, 0
};
	
void delay_us(uchar n_usec);
void delay_ms(uint n_msec);
void play_a_song(struct music_type notetab[]);

void main(void)
{
	TMOD = 0x01; //use timer0 mode 1 to generate tone
	IE = 0x83; //enable timer0 overflow interrupt and external interrupt
	chg_song = 1;
	while (1) {
		chg_song = 1;
		
		if (song1)
			play_a_song(maryhadalittlelamb);
		else if (song2)
			play_a_song(twinkletwinkle);
		else if (song3)
			play_a_song(happybirthday);
		else if (song4)
			play_a_song(jinglebells);
		else 
			TR0 = 0;
	}
}

void play_a_song(struct music_type notetab[])
{
	uchar i, j;
	while (chg_song)
	{
		i = 0;
		while (notetab[i].delay&&chg_song)
		{
			if (!notetab[i].tone)
				TR0 = 0; //stop tone, turn off timer0
			else
			{
				th0 = TH0 = tonetab_C[notetab[i].tone-1] >> 8; //get higher 8-bit of the initial value
				tl0 = TL0 = tonetab_C[notetab[i].tone-1]; //get lower 8-bit of the initial value
				TR0 = 1;
			}
			
			//provide duration for each tone
			for (j=0; j<notetab[i].delay; j++)
				delay_ms(1000); //minimum delay unit for 1/16
			
			i++;
		}
		TR0 = 0; //stop between songs for about 0.2s
		delay_ms(2000);
		
	}		
}

void delay_us(uchar n_usec)
{
	do
	{
		_nop_();
		_nop_();
		_nop_();
		_nop_();
		_nop_();
	} while (--n_usec);
}

void delay_ms(uint n_msec)
{
	do
		delay_us(131);
	while (--n_msec);
}

void T0_ISR(void) interrupt 1 //timer0 interrupt service routine
{
	TR0 = 0;
	Buzzer = !Buzzer;
	TH0 = th0;
	TL0 = tl0;
	TR0 = 1;
}

void change_song(void) interrupt 0
{
	chg_song = 0;
}



	
	
	
	
	
	
	
	
	
	
	