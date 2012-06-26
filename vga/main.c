//Simplest universal VGA(20x20)/ terminal
//For sync used Timer0 Timer1
//To avoid flickering while receive UART data,
//recommend to send UART data after VSYNC during
//10-20 VGA lines timing (around 300-600 mks)..

#include <avr/io.h>
#include <avr/signal.h>
#include <avr/sleep.h>
#include <avr/pgmspace.h>
#include <string.h>
#include "symbol_tbl.h" 

#define _BV(bit) (1 << (bit))
#define PRG_RDB pgm_read_byte 


#define true 1
#define false 0
#define NOP asm("nop")

//Global definitions for VGA render
#define vga_field_line_count 525 //standart VGA quantity lines
#define vga_symbols_per_row 20   //symbols quantity per horizontal
#define vga_row_count 20         //symbols quantity per vertical
#define vga_symbol_height 24	 //rendered symbol height

//Makro for video handle
//Video
#define video_off  DDRB=0x90
#define video_on  DDRB=0xB0
//HSYNC
#define hsync_off  PORTD|=_BV(PD3);//sbi(PORTD,3)
#define hsync_on  PORTD&=~(_BV(PD3));//cbi(PORTD,3)
//VSYNC
#define vsync_off  PORTD=0x04
#define vsync_on   PORTD=0x00

//Global variables
volatile unsigned char str_array[pal_symbols_per_row*pal_row_count+1]; //Chars array for display buffer
volatile unsigned int current_symbol; //Current symbol number at terminal

volatile unsigned char video_enable_flg;//flag for enable render
volatile unsigned char raw_render;//Current row at display buffer for render
volatile unsigned int linecount;// current line for render
volatile unsigned char y_line_render;	//Current Y-line at symbol for render 

//Store strings data at Program Space, to avoid RAM leakage
const char str1[] PROGMEM =   "Smart Notice Board System";
const char str2[] PROGMEM =   "Smart Notice Board ";
const char str3[] PROGMEM =   "GEC Sreekrishnapuram,AXIS";

static void avr_init(void);

//All VGA sincronize made here..
SIGNAL(SIG_OVERFLOW0)
{
 TCNT0 = 0xC3; //reload counter value
		//******Syncronization Handler********

			//Count number of lines
			if (++linecount == vga_field_line_count)
			{
			linecount = 0;
			
			//clear pointers for render display buffer
			raw_render = 0; 
			y_line_render = 0;
			}

			//Make Vsync length 2 VGA lines 
			if ((linecount == 10 )||(linecount == 11 ))
			{
				//Make here vertical syncronization & HSYNC syncro level on
				vsync_on;
			}
			else
			{
				//.. & HSYNC syncro level on
				vsync_off;
			}
		
		
			video_enable_flg = true;


			if (linecount < 45)
			{
				video_enable_flg = false;
					//Add to avoid flickering at top display
					NOP;
					NOP;
					NOP;
					NOP;
					NOP;
					NOP;
					NOP;
					NOP;
					NOP;
					NOP;
					NOP;
					NOP;
					NOP;
					NOP;
					NOP;
					
			}
			else
			{
				 //Forming current string for rendering
				 if (++y_line_render == vga_symbol_height)
				 {
				  raw_render++;
				  y_line_render = 0;
				 }
				 else
				 {
					NOP;
					NOP;
					NOP;
					NOP;
					NOP;
					NOP;
					NOP;
					NOP;
				 }
		
			}
		
			hsync_off; //HSYNC syncro level off
		
		//******Syncronization Handler********

}


void spi_init (void)
{
    //Set SPI PORT DDR bits
	
DDRB|=_BV(DDB7);
DDRB|=_BV(DDB5);
DDRB|=_BV(DDB4);
DDRB&=~(_BV(DDB6));
	SPSR = 1 << SPI2X;
	SPCR = (1 << SPE) | (1 << MSTR); //SPI enable as master ,FREQ = fclk/2
	//That's a great pity, that we can't work with SPI with FREQ = fclk,
	//because may be possible build terminal up 40 symbol per row!!!
}

static void uart_init(void)
{
	 UCSRB = 0x00; //disable while setting baud rate
	 UCSRA = 0x00;
	 UCSRC = 0x86;	//8 bits data & 2 stop bits
	//UART SPEED 19200 bs
	UBRRL = 0x33; //set baud rate lo
	UBRRH = 0x00; //set baud rate hi
	
	UCSRB = (1 << RXEN);
}


static void vga_terminal_handle(void)
{
	// Parser received symbols from UART
	while(UCSRA & (1<<RXC))
	{
		unsigned char current_line, received_symbol;
		received_symbol = UDR;
		//Check for overflow display buffer
		if(current_symbol == (vga_row_count*vga_symbols_per_row))
		{
			
			//Clear display buffer

			unsigned char i = vga_row_count;
			char * ptr;
			//Set pointers for clear string array
			ptr = &str_array[0];
			while(i--)
			{
				//Don't use here loop, to fastest clear display buffer
				*ptr++ = 0x0;
				*ptr++ = 0x0;
				*ptr++ = 0x0;
				*ptr++ = 0x0;
				*ptr++ = 0x0;
				*ptr++ = 0x0;
				*ptr++ = 0x0;
				*ptr++ = 0x0;
				*ptr++ = 0x0;
				*ptr++ = 0x0;
				*ptr++ = 0x0;
				*ptr++ = 0x0;
				*ptr++ = 0x0;
				*ptr++ = 0x0;
				*ptr++ = 0x0;
				*ptr++ = 0x0;
				*ptr++ = 0x0;
				*ptr++ = 0x0;
				*ptr++ = 0x0;
				*ptr++ = 0x0;
			};
			current_symbol = 0x0;
		}
					switch ( received_symbol )
					{
					  //BackSpace
					  case  0x08: 
					  if(current_symbol)
					    {
							str_array[current_symbol] = 0x0;
							str_array[--current_symbol] = 0x0;
						}
					  break;
					  
					  //TAB
					  case  0x09: 
					  if((current_symbol + 5) < (vga_row_count*vga_symbols_per_row))
					    {
							//Add 5 Space
							str_array[current_symbol] = 0x0;
							current_symbol += 5;
						}
					  break;

					  //RETURN
					  case  0x0D: 
					  current_line = current_symbol / vga_symbols_per_row;
					  if((current_line) < 19)
					    {
							str_array[current_symbol] = 0x0;
							current_symbol = current_line*vga_symbols_per_row + vga_symbols_per_row;
						}
					  break;
					  default: str_array[current_symbol++] = received_symbol;
					}
	}
}

void vga_render()
{
	unsigned char i;
	char * _ptr;
	const char * _ptr1;

	//Initialize display buffer with StartUp strings
	strcpy_P(&str_array[vga_symbols_per_row*(vga_row_count-2)],&str2[0]); 	
	strcpy_P(&str_array[vga_symbols_per_row*(vga_row_count-1)],&str3[0]); 	

	//Enable global interrupt
	asm ("sei");

	for(;;)
    {

		//Wait compare interrupt signal from Timer1
		sleep_mode();
		
		//Check symbol on UART
		if (UCSRA & (1<<RXC))
		{
			//Parse received symbol
			vga_terminal_handle();
			continue;
		}
		
		//Check visible field
		if(video_enable_flg)
		{
		//OK, visible
		//Main render routine
				//Set pointer for render line (display bufffer)
				_ptr = &str_array[raw_render * vga_symbols_per_row];

				//Set pointer for render line (character generator)
				_ptr1 = &symbol[0][y_line_render >> 1];
	
				//Cycle for render line 
				i = vga_symbols_per_row;
				while(i--)
				{
					SPDR = PRG_RDB(_ptr1 + (* _ptr++)*vga_symbol_height/2);
				    video_on;
					//That's a great pity can't shift data faster (8Mhz at FOSC=16Mhz)!!
					NOP;
					NOP;
				}
				//Delay for draw last symbol
				NOP;
				NOP;
				NOP;
				NOP;
				NOP;
				NOP;
				video_off;

		}
		else
		{
		//Not visible
		//Can do something else..	
					//******Cursor handle
					//Count frame
					static unsigned int framecount;
					framecount++;
					//Here draw cursor
						if (framecount&0x800)
						{
							str_array[current_symbol] = ' ';
						}
						else
						{
							str_array[current_symbol] = 0x7F;
						}
					//******Cursor handle
		//You can add here your own handlers..			
		}
		
    }//for(;;)
}

int main(void)
{
    avr_init();
	  //Check pin VGA/PAL
	
	vga_render();
	
    return(0);
}



static void avr_init(void)
{

  //Set pin jumper VGA/PAL
  //cbi(DDRC,1);
  //sbi(PORTC,1);
  
DDRC&=~(_BV(DDC1));
PORTC|=_BV(PC1);

  //Enable SPI
  spi_init();

  //init uart
  uart_init();

  //Set power mode
  set_sleep_mode(SLEEP_MODE_IDLE); 	

 

	  //init VGA SYNC ports
	  //HSYNC
	  PORTD|=_BV(PD3);
	  DDRD|=_BV(DDD3);
	  //VSYNC
	  
          PORTD|=_BV(PD2);
	  DDRD|=_BV(DDD2);
	
	  // Initialize Sync for VGA
	 TCCR0 = 0x00; //stop
	 TCNT0 = 0xC3; //set count, One VGA line 31.77us
	 TCCR0 = 1<<CS01; //start timer with prescaler select 1/8
	 TIMSK = 1<<TOV0; //enable interrupt from Timer0 overflow
  

  return;
}
    
	
    
