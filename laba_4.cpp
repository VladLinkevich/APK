#include <stdio.h>
#include <conio.h>
#include <dos.h>

#define COUNT 9

#define DELAY 10
#define TIME_RUN 65536

void soundGeneration();
void divisionCoefficient();
void statusWord();
void randomNumber();

void main() {
	clrscr();  //очищаем экран
	char choice;
	do {
		printf("\n\n1. Sound generation.\n");
		printf("2. Coefficient of division.\n");
		printf("3.Status word.\n");
		printf("4.Random number.\n");
		printf("0.Exit.\n");
		fflush(stdin);
		printf("\nYour choise: ");
		scanf("%s", &choice);

		switch (choice) {
		case '1': soundGeneration(); break;
		case '2': divisionCoefficient(); break;
		case '3': statusWord(); break;
		case '4': randomNumber(); break;
		}
	} while (choice != '0');
}

void soundGeneration() {
	
	int frequency[COUNT] = { 392, 392, 293, 196, 196, 392, 392, 293, 196};   //массив частот звуков
	int durability[COUNT] = { 800, 400, 400, 400, 400, 400, 400, 400, 400};  //массив длительности в мс
	
	long unsigned base = 1193180;              //тактовая частота синхронизации для пк
	                                           //она одинакова для всех трех каналов
	int frequencyCounter;
	int kd;         //коэффициент деления

	for (frequencyCounter = 0; frequencyCounter < COUNT; frequencyCounter++) {
		outp(0x43, 0xB6);                   //настроить канал 2 (0В6h = 10110110b)
		                                    //B6 = 10 - канал2,
		                                    //11-чтение/запись сначала младшего,а затем старшего байта,
											//011 -режим 3, (счетчик уменьшается на два за каждый такт синхронизации
											//При счеттчике == 0 или переполнении состояние меняется на противоноложное
											//а счет начинается с начала)
											//0 - двоичный счет
		kd = base / frequency[frequencyCounter];    //коэффициент деления = частота синхронизации/частота звука
		//загрузим значение счетчика побайтово
		outp(0x42, kd % 256);   //младший байт(берем остаток от деления на 256, 
		                        //так как это максимальное число ,которое помещается в 1 байте)
		
		outp(0x42, kd/256);     //старший байт(берем целую часть от деления на 256)

		outp(0x61, inp(0x61) | 3);   //для выдачи звукового сигнала
									 //два младших бита порта 61h должны быть установлены в 1(3h=11b)
									 //побитовое ИЛИ исходного значения порта с 11b гарантирует ,что 2 младших бита равны единицам
									 //нулевой бит -вход канала 2,разрешает счет
								     //первый бит-разрешение выдачи выхода OUT на вход звукогенератора
		delay(durability[frequencyCounter]);     //пауза
		outp(0x61, inp(0x61) & 0xFC);     //(0FCh=11111100b)зануляем два младших бита чтобы прекратить выдачу звукового сигнала
		
		
	}
}

void divisionCoefficient() {
	unsigned long j;
	int Channel;
	int ports[] = { 0x40, 0x41, 0x42 };   //личные порты каналов
	int controlWords[] = { 0, 64, 128 }; //байт передающийся в регистр управления
	                                     //соответствено для каналов 0,1,2
			//0d = 00_00_000_0b
			//xx(номер канала)_00(фиксация текущего значения счетчика)_000(прерывание по счетчику)_0(двоичный)
			//64d =01_00_000_0b 
			//128d=10_00_000_0b
	unsigned kdLow, kdHigh, kd, max;

	printf("\n\nCoefficient of division: \n");

	for (Channel = 0; Channel < 3; Channel++) {
		kd = 0;
		max = 0;

		

		for (j = 0; j < TIME_RUN; j++) {
			outp(0x43, controlWords[Channel]);
			kdLow = inp(ports[Channel]);    //считываем значение младшего байта с порта канала
			kdHigh = inp(ports[Channel]);   //...старшего байта...
			kd = kdHigh * 256 + kdLow;      //получаем коэффициент деления

			if (kd > max) {
				max = kd;
			}
		}
		
		printf("\nChannel %d: %4X\n", Channel, max);    //выводим значение в 16 ричном виде
	}
}

void statusWord() {
	printf("\n\nStatus word: \n");
	char stateWord[8];  //сюда записываем наше число в двоичном виде
	int Channel, state;
	int ports[] = { 0x40, 0x41, 0x42 };
	int controlWords[] = { 226, 228, 232 };   //11 1 0 001 0b, 11 1 0 010 0b, 11 1 0 100 0b
	//11(код команды считывания)_1(значения выбранных счетчиков не фиксируются)
	//0(состояния выбранных счетчиков фиксируются для последующего считывания)
	//xxx(выбираем какой канал будет зафиксирован)
	//0 -зарезервирован
	for (Channel = 0; Channel < 3; Channel++) {
		outp(0x43, controlWords[Channel]);   //загружаем управляющий байт
		state = inp(ports[Channel]);   //считываем состояние

		for (int i = 7; i >= 0; i--) {
			stateWord[i] = state % 2 + '0';  //переводим в двоичное число
			state /= 2;

		}
		printf("\nChannel %d: ", Channel);
		for (i = 0; i < 8; i++) {
			printf("%c", stateWord[i]);   //выводим двоичное число
		}
		printf("\n");
	}
}

void randomNumber() {
	char choice;
	unsigned int limit = TIME_RUN - 1, numberLow, numberHigh, number;

	do {
		printf("\n\n1. Set a limit.\n2. Get a number.\n0. Exit\n");
		fflush(stdin);
		printf("\nYour choise: ");
		scanf("%s", &choice);

		switch (choice) {
		case '1':
			do {
				printf("\nEnter a limit in range [1...65635].\n");
				fflush(stdin);
			} while (!scanf("%d", &limit) || limit < 1);

			outp(0x43, 0xB4); //0B4h =10_11_010_0(канал,
			                  // чтение/запись поочередно 2 байт,режим 2,двоичный)
			outp(0x42, limit % 256);
			limit /= 256;
			outp(0x42, limit);   //заносим побайтово лимит
			outp(0x61, inp(0x61) | 1);  //разрешаем начать счет(но при этом блокируем звук)

			break;

		case '2':
			outp(0x43, 128);  //128=
			//10(номер канала)_00(фиксация текущего значения счетчика)_000(прерывание по счетчику)_0(двоичный)
			numberLow = inp(0x42); //считываем значение младшего байта
			numberHigh = inp(0x42);  //старшего
			number = numberHigh * 256 + numberLow;  //получаем число

			printf("\nRandom number: %u\n", number);

			break;
		}
	} while (choice != '0');
	outp(0x61, inp(0x61) & 0xFC);
}