#include <dos.h>
#include <stdio.h>
#include <conio.h>
#include <io.h>

void PLAY_SOUND() {
	
_asm {
	mov bx, 0
	mov ax, 0E07h
	int 10h
}

};

unsigned int timerCount = 0;

void interrupt(*timeOld)(...);


int milsec = 0;

void interrupt timeNew(...) {

milsec++;
outp(0x70, 0x0C);
inp(0x71);
//Посылаем контроллерам прерываний сигнал EOI
outp(0x20, 0x20);
outp(0xA0, 0x20);

}

void interrupt(*oldAlarm) (...);

struct VIDEO {
unsigned char symb;
unsigned char attr;
};

const char* alarmClockText[] = { "Wake up!", "Wake up!" };
const int alarmClockTextSize[] = { 8, 8 };

void interrupt newAlarm(...) {

	clrscr();
	VIDEO far* screen = (VIDEO far*)MK_FP(0xB800, 0);
	screen += 30;
	for (int l = 0; l < 2; l++) {
		for (int h = 0; h < alarmClockTextSize[l]; h++) {
			
			screen->symb = alarmClockText[l][h]; screen->attr = 12; screen++;
			delay(50);
		}

	screen += 55;
	}

}

void reset()
{

	if (oldAlarm != NULL) {

		setvect(0x4A, oldAlarm);
		outp(0xA1, (inp(0xA0) | 0x01)); // запретить прерывание часов реального времени
		outp(0x70, 0x05); //часы будильника
		outp(0x71, 0x00); //сброс настроек
		outp(0x70, 0x03); //минуты будильника
		outp(0x71, 0x00); //сброс настроек
		outp(0x70, 0x01); //секунды будильника
		outp(0x71, 0x00); //сброс настроек
		outp(0x70, 0xB);

		outp(0x71, (inp(0x71) & 0xDF)); // блокировать прерывание
	}
}

int bcdToDecimal(int number){
	return (((number / 16) * 10) + (number % 16));
}

int decimalToBCD(int number){
	return ((number / 10 * 16) + (number % 10));
}

void getTime(){
	
//порт 70h задаёт номер регистра, а порт 71h используется для чтения и записи
//выбрать регистр через порт 70h
//считать значение
	outp(0x70, 0x04); //часы
	printf("%02d:", bcdToDecimal(inp(0x71)));//%02d- вывод целочисленных значений в 2 или более числах, первое =0, если число меньше или равно 9
	outp(0x70, 0x02); //минуты
	printf("%02d:", bcdToDecimal(inp(0x71)));
	outp(0x70, 0x00); //секунды
	printf("%02d\n", bcdToDecimal(inp(0x71)));

}

void getTimeAlarm(){
//порт 70h задаёт номер регистра, а порт 71h используется для чтения и записи
//выбрать регистр через порт 70h
//считать значение
	outp(0x70, 0x05); //часы
	printf("%02d:", bcdToDecimal(inp(0x71)));//%02d- вывод целочисленных значений в 2 или более числах, первое =0, если число меньше или равно 9
	outp(0x70, 0x03); //минуты
	printf("%02d:", bcdToDecimal(inp(0x71)));
	outp(0x70, 0x01); //секунды
	printf("%02d\n", bcdToDecimal(inp(0x71)));

}

void getDate(){
//выбрать регистр через порт 70
//считать значение
	outp(0x70, 0x07); //день месяца
	printf("%02d.", bcdToDecimal(inp(0x71)));
	outp(0x70, 0x08); //месяц
	printf("%02d.", bcdToDecimal(inp(0x71)));
	outp(0x70, 0x09); //год 2 младшие цифры
	printf("%02d\n", bcdToDecimal(inp(0x71)));

}

void setTime(int num){

	int hours;
	rewind(stdin);
	printf("Hours: ");
	scanf("%i", &hours);//%i-ожидает на входе строку с с целым числом в 10, 8(начиная с 0) или 16(0х)системах счисления
	while (hours > 23 || hours < 0) {

		rewind(stdin);
		printf("Hours: ");
		scanf("%i", &hours);//%i-ожидает на входе строку с с целым числом в 10, 8(начиная с 0) или 16(0х)системах счисления
	}	

	hours = decimalToBCD(hours);
	int minutes;
	rewind(stdin);
	printf("Minutes: ");
	scanf("%i", &minutes);

	while (minutes > 59 || minutes < 0) {

		rewind(stdin);
		printf("Minutes: ");
		scanf("%i", &minutes);
	}

	minutes = decimalToBCD(minutes);
	int seconds;
	rewind(stdin);
	printf("Seconds: ");
	scanf("%i", &seconds);

	while (seconds > 59 || seconds < 0) {

		rewind(stdin);
		printf("Seconds: ");
		scanf("%i", &seconds);
	}

	seconds = decimalToBCD(seconds);
	if (num)
	{

		unsigned int result;
		//проверяем доступность значений регистров для чтения/записи
		//ждём, пока старший бит регистра состояния А не станет равным 0

		do {

			outp(0x70, 0x0A);//регистр А задаёт частоту срабатывания прерывания
			result = (inp(0x71) & 0x80);

		} while (result);

		outp(0x70, 0xB);//регистр В
		outp(0x71, inp(0x71) | 0x80); // запрет обновления
		outp(0x70, 0x04);//установить часы
		outp(0x71, hours);
		outp(0x70, 0x02);//установить минуты
		outp(0x71, minutes);
		outp(0x70, 0x00);//установить секунды
		outp(0x71, seconds);
		outp(0x70, 0xB);
		outp(0x71, inp(0x71) & 0x7F); // разрешено обновление

	} else {

		_disable();
		outp(0xA1, (inp(0xA1) & 0xFE));
		unsigned int res;

			do {

				outp(0x70, 0xA);
				res = inp(0x71) & 0x80;

			} while (res);

			outp(0x70, 0x05);
			outp(0x71, hours);//установить часы будильника
			outp(0x70, 0x03);
			outp(0x71, minutes);//установить минуты будильника
			outp(0x70, 0x01);
			outp(0x71, seconds);//установить секунды будильника

			//Разрешаем прерывание будильника установив 1 в 5 бит регистра состояния B

			outp(0x70, 0xB);
			outp(0x71, (inp(0x71) | 0x20));
			//Переопределяем вектор прерывания

			oldAlarm = getvect(0x4A);
			setvect(0x4A, newAlarm);
			_enable();

	}
}

void setDelay() {

	unsigned long delayPeriod;
	unsigned char value;
	_disable();
	timeOld = getvect(0x70);
	setvect(0x70, timeNew);
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
	setvect(0x70, timeOld);


}

char check() {

	int choice = 0;
	int res = scanf("%d", &choice);
	rewind(stdin);

	while (res < 1 || choice < 0 || choice > 3) {

		printf("repeat input\n");
		rewind(stdin);
		res = scanf("%d", &choice);

	}

	return (char)choice + '0';
}

int main()
{

	printf("1.Set and get time\n");
	printf("2.Sleep function\n");
	printf("3.Set alarm\n");	
	printf("0.Exit\n");
	printf("Input, please\n");

	char choice = check();

	while (choice != '0') {

	switch (choice)
	{

		case'1':
		printf("Current date: ");
		getDate();
		printf("Current time: ");
		getTime();
		printf("Set time: ");
		setTime(1);
		printf("Current date: ");
		getDate();
		printf("Current time: ");
		getTime();
		printf("Set time: "); break;

	case'2':
		setDelay();
		break;

	case'3':
		setTime(0);
		getTimeAlarm();
		break;

	case'0': reset(); return 1;

	}

		printf("if you want exit, input 0\n");
		choice = check();

	}
return 0;

}