#include <dos.h>
#include <ctype.h>
#include <stdio.h>
#include <conio.h>
#include <string.h>
#include <stdlib.h>

int milsec = 0;

void interrupt (*oldI70h)(...);

void interrupt newI70h(...){
	milsec++;
	outp(0x70,0x0C);
	inp(0x71);
	outp(0x20,0x20);
	outp(0xA0,0x20);
}

void interrupt (*oldI4Ah)(...);

void interrupt newI4Ah(...){  
	
	oldI4Ah();  

	outp(0x43, 0xB6);

	long unsigned value = 1193180/400;
	outp(0x42, (unsigned char)value);
	outp(0x42, (unsigned char)(value >> 8));

	outp(0x61, (inp(0x61)|3));
	delay(5000);
	outp(0x61, inp(0x61) & 0xFC);

	outp(0x20,0x20);
}

unsigned char get_param(int reg)
{
	unsigned char result;
	outp(0x70, reg);	
	result = inp(0x71);					
	return result;
}

void wait(){ 
	do{
		outp(0x70, 0x0A);
	}while(inp(0x71) & 0x80 );
}

void disableUpdate(){  
    wait();
	outp(0x70, 0x0B);
	outp(0x71, (inp(0x71) | 0x80));
	printf("Time update was disabled\n");
}

void enableUpdate(){  
	wait();
    outp(0x70, 0x0B);
    outp(0x71, (inp(0x71) & 0x7F));
	printf("Time update was enabled\n");
}

int inputInt(int min, int max){
	int number;
	do{
		fflush(stdin);
	}while(!scanf("%d", &number) || number<min || number>max);
	return number;
}

int getInt(int BCD){
	return BCD % 16 + BCD / 16 * 10;
}

unsigned char getBCDcode(int IntNumber){
	 return (unsigned char)((IntNumber/10)<<4)|(IntNumber%10);
}

void set_param(int value,int reg,int min, int max)
{
	value = inputInt(min,max);
	outp(0x70, reg);	
	outp(0x71, getBCDcode(value));					
}

void getTotal()
{
	char months[13][12] = {{"Unknown"},{"January"},{"February"},{"March"},{"April"},{"May"},{"June"},{"July"},{"August"},{"September"},{"October"},{"November"},{"December"}};
	char weekdays[8][12] = {"Unknown","Sunday","Monday","Tuesday","Wednesday","Thursday","Friday","Saturday"};


	unsigned char hour,minute,second,year,month,day,weekday;
	
	disableUpdate();
	printf("The result of data and time:\n");
	hour = get_param(0x04);							
	minute = get_param(0x02);						
	second = get_param(0x00);						
	year = get_param(0x09);							
	month = get_param(0x08);						
	day = get_param(0x07);							
	weekday = get_param(0x06);						
	printf("Time:	%d:%d:%d\nData:		%s %d,20%d	-	%s\n", getInt(hour),getInt(minute),getInt(second),months[getInt(month)],getInt(day),getInt(year),weekdays[getInt(weekday)]);
	enableUpdate();
}

void setTotal()
{
	int value;
    disableUpdate();
	printf("\nHour: ");
	set_param(value,0x04,0,24);
	printf("\nMinutes: ");
	set_param(value,0x02,0,60);
	printf("\nSeconds: ");
	set_param(value,0x00,0,60);
	printf("\nDay of week: " );
	set_param(value,0x06,1,7);
	printf("\nDay of month: ");
	set_param(value,0x07,1,31);
	printf("\nMonth: ");
	set_param(value,0x08,1,12);
	enableUpdate();
    printf("Time is successfully set\n");
}

void delayTime(){
	unsigned long delayPeriod;
	unsigned char value;
	_disable();
	oldI70h = getvect(0x70);
	setvect(0x70, newI70h);
	_enable();
	printf("\nEnter delay time in milliseconds: ");
	scanf("%ld", &delayPeriod);
	printf("Delaying ...");
	value= inp(0xA1) & 0xFE;
	outp(0xA1,value);
	outp(0x70, 0x0B);
	value= inp(0x71) |0x40;
	outp(0x70, 0x0B);
	outp(0x71, value);
	milsec = 0;
	while(milsec!= delayPeriod);
	printf("\nEnd delay of %d ms\n", milsec);
	setvect(0x70, oldI70h);
	enableUpdate();
}

void setAlarm()
{
    int value;
    disableUpdate();     // disable clock update
    printf("Enter hour: ");
	value = inputInt(0,24);
    outp(0x70, 0x05);
    outp(0x71, getBCDcode(value));
    printf("Enter minute: ");
    value = inputInt(0,60);
    outp(0x70, 0x03);
    outp(0x71, getBCDcode(value));
    printf("Enter second: ");
    value = inputInt(0,60);
    outp(0x70, 0x01);
    outp(0x71, getBCDcode(value));
	outp(0x70, 0x0B);
	outp(0x71, (inp(0x71)| 0x20));
    enableUpdate(); // enable clock update
    printf("Alarm is set\n");
}

void viewAlarm()
{
    unsigned char value;
    disableUpdate();     // disable clock update

    outp(0x70, 0x05);
    value = inp(0x71);
	printf("Alarm - %d:", getInt(value));

    outp(0x70, 0x03);
    value = inp(0x71);
	printf("%d:", getInt(value));

    outp(0x70, 0x01);
    value = inp(0x71);
	printf("%d\n", getInt(value));
    enableUpdate(); // enable clock update
}

int main()
{
	char choose, value;
    oldI4Ah = getvect(0x4A);
    setvect(0x4A, newI4Ah);
    while(choose != '0'){
    clrscr();
    printf("\n1)Time info\n2)Set time\n3)Set alarm time\n4)Delay time(alarm clocks set)");
    printf("\n5)Show alarm clocks\n0)Leave away\n");
	choose = getch();
		switch(choose){
			case '1': getTotal(); break;
			case '2': setTotal(); break;
			case '3': delayTime(); break;
			case '4': setAlarm(); break;
			case '5': viewAlarm(); break;
			case '0': break;
		}
		getch();
    }
	setvect(0x4A, oldI4Ah);
	return 0;
}