#include <dos.h>
#include <stdio.h>
#include <stdlib.h>

char color = 0x6E;  

struct VIDEO
{
  unsigned char symb; 
  unsigned char attr;
};


void get_reg(); 
void print(int, int, int); 
void printstr(int, int, int);



void interrupt(*old_0x08) (...);
void interrupt(*old_0x09) (...);
void interrupt(*old_0x0A) (...);
void interrupt(*old_0x0B) (...);
void interrupt(*old_0x0C) (...);
void interrupt(*old_0x0D) (...);
void interrupt(*old_0x0E) (...);
void interrupt(*old_0x0F) (...);

void interrupt(*old_0x70) (...);
void interrupt(*old_0x71) (...);
void interrupt(*old_0x72) (...);
void interrupt(*old_0x73) (...);
void interrupt(*old_0x74) (...);
void interrupt(*old_0x75) (...);
void interrupt(*old_0x76) (...);
void interrupt(*old_0x77) (...);


void interrupt  new_0x70(...) { get_reg(); old_0x70(); }
void interrupt  new_0x71(...) { get_reg(); old_0x71(); }
void interrupt  new_0x72(...) { get_reg(); old_0x72(); }
void interrupt  new_0x73(...) { get_reg(); old_0x73(); }
void interrupt  new_0x74(...) { color++; get_reg(); old_0x74(); }
void interrupt  new_0x75(...) { get_reg(); old_0x75(); }
void interrupt  new_0x76(...) { get_reg(); old_0x76(); }
void interrupt  new_0x77(...) { get_reg(); old_0x77(); }

void interrupt  new_0x08(...) { get_reg(); old_0x08(); }
void interrupt  new_0x09(...) { color++; get_reg(); old_0x09(); } 
void interrupt  new_0x0A(...) { get_reg(); old_0x0A(); }
void interrupt  new_0x0B(...) { get_reg(); old_0x0B(); }
void interrupt  new_0x0C(...) { get_reg(); old_0x0C(); } 
void interrupt  new_0x0D(...) { get_reg(); old_0x0D(); }
void interrupt  new_0x0E(...) { get_reg(); old_0x0E(); }
void interrupt  new_0x0F(...) { get_reg(); old_0x0F(); }



void print(int val, int y, int x)
{
      char temp;
      VIDEO far* screen = (VIDEO far *)MK_FP(0xB800, 0);
      screen += ( y * 80 + x );
	  
	  int i;
	  for(i = 7; i >= 0; i--) {    
		temp = val&1;
		val /=2;
		screen->symb = temp + '0';
		color &= 15;
		screen->attr = color;
		screen++;
      }
}



void printstr(int num, int y, int x)
{
	char str1[] = {'M', 'A', 'S', 'T', 'E', 'R', ' ', ' ', ' ', 'I', 'R', 'R', '\0'};
	char str2[] = {'I', 'S', 'R', '\0'};
	char str3[] = {'I', 'M', 'R', '\0'};
	char str4[] = {'S', 'L', 'A', 'V', 'E', ' ', ' ', ' ', ' ', 'I', 'R', 'R', '\0'};
	int i;
	VIDEO far* screen = (VIDEO far *)MK_FP(																									0xB800, 0);
	screen += ( y * 80 + x );
	switch (num)
	{
		case 1:
		{
			for(i = 0; str1[i] != 0; i++)
			{
				screen->symb=str1[i];
				screen->attr= 0x04;
				screen++;
			}
			break;
		}
	       case 2:
		{
			for(i = 0; str2[i] != 0; i++)
			{
				screen->symb=str2[i];
				screen->attr= 0x04;
				screen++;
			}
			break;
		}
		case 3:
		{
			for(i = 0; str3[i] != 0; i++)
			{
				screen->symb=str3[i];
				screen->attr= 0x04;
				screen++;
			}
			break;
		}
		case 4:
		{
			for(i = 0; str4[i] != 0; i++)
			{
				screen->symb=str4[i];
				screen->attr= 0x04;
				screen++;
			}
			break;
		}
	}
}


void get_reg()
{
	//получаем данные для регистра запросов ВЕДУЩЕГО и печатаем
	printstr(1, 13, 25);
	outp(0x20, 0x0A);
	print(inp(0x20), 14, 34);
	
	//получаем данные для регистра обслуживания ВЕДУЩЕГО
	printstr(2, 15, 34);
	outp(0x20, 0x0B);
	print(inp(0x20), 16, 34);
	
	//р-р масок ВЕДУЩЕГО
	printstr(3, 17, 34);
	print(inp(0x21), 18, 34);

	//все тоже для ведомого
	printstr(4, 19, 25);
	outp(0xA0, 0x0A);
	print(inp(0xA0), 20, 34);
	printstr(2, 21, 34);
	outp(0xA0, 0x0B);
	print(inp(0xA0), 22, 34);
	printstr(3, 23, 34);
	print(inp(0xA1), 24, 34);
}



void init()
{

	old_0x08 = getvect(0x08);
	old_0x09 = getvect(0x09);
	old_0x0A = getvect(0x0A);
	old_0x0B = getvect(0x0B);
	old_0x0C = getvect(0x0C);
	old_0x0D = getvect(0x0D);
	old_0x0E = getvect(0x0E);
	old_0x0F = getvect(0x0F);

	old_0x70 = getvect(0x70);
	old_0x71 = getvect(0x71);
	old_0x72 = getvect(0x72);
	old_0x73 = getvect(0x73);
	old_0x74 = getvect(0x74);
	old_0x75 = getvect(0x75);
	old_0x76 = getvect(0x76);
	old_0x77 = getvect(0x77);

	setvect(0x70, new_0x08);
	setvect(0x71, new_0x09);
	setvect(0x72, new_0x0A);
	setvect(0x73, new_0x0B);
	setvect(0x74, new_0x0C);
	setvect(0x75, new_0x0D);
	setvect(0x76, new_0x0E);
	setvect(0x77, new_0x0F);

	setvect(0x08, new_0x70);
	setvect(0x09, new_0x71);
	setvect(0x0A, new_0x72);
	setvect(0x0B, new_0x73);
	setvect(0x0C, new_0x74);
	setvect(0x0D, new_0x75);
	setvect(0x0E, new_0x76);
	setvect(0x0F, new_0x77);


	_disable(); 

	//Ведущий
	outp(0X20, 0x11);	
	outp(0x21, 0x70);   
	outp(0x21, 0x04); 	
	outp(0X21, 0x01);	

	//Ведомый
	outp(0xA0, 0x11);	
	outp(0xA1, 0x08);	
	outp(0xA1, 0x02);	
	outp(0xA1, 0x01);       

	_enable(); 

}

int main()
{
      unsigned far *fp;
      init();

      FP_SEG (fp) = _psp; 
      FP_OFF (fp) = 0x2c; 
					
      _dos_freemem(*fp);

      _dos_keep(0, (_DS -_CS) + (_SP/16) + 1);
	return 0;
}