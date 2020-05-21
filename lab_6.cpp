#include <dos.h>
#include <stdio.h>
#include <io.h>
#include <stdlib.h>
unsigned int blinking = 0;
int quitFlag = 0;
unsigned int repeat = 1;
unsigned int attempt = 3;

void interrupt(*oldInt09)(...);
void interrupt newInt09(...)
{
	unsigned char keyCode;

	keyCode = inp(0x60);
	if (keyCode == 0x01)
	{
		quitFlag = 1;
		return;
	}
	
	if (keyCode == 0xFE && blinking)//FE – ïðîèçîøëà îøèáêà,åñòü 3 ïîïûòêè
	{
		if (attempt-- == 0)
		{
			printf("Error. Input register of keyboard is busy.\n");
			quitFlag = 1;
		}
		repeat = 1;
	}
	else
	{
		repeat = 0;
		attempt = 3;
	}

	printf("\t%x", keyCode);
	outp(0x20, 0x20); // Ïîñûëàåì êîíòðîëëåðó ïðåðûâàíèé (master) ñèãíàë EOI (end of interrupt)
	outp(0xA0, 0x20);// Ïîñûëàåì âòîðîìó êîíòðîëëåðó ïðåðûâàíèé (slave) ñèãíàë EOI (end of interrupt)
}

void keyBlinking(unsigned char i)
{
	int timeout = 50000;
	repeat=1;
	while (repeat)
	{
		//ïðîâåðÿåì íàëè÷èíå äàííûõ âî âõîäíîì áóôåðå êëàâèàòóðû
		while (timeout-- > 0)
		{
			//Òàê êëàâèàòóðà ðàáîòàåò ìåäëåííî, çàïèñü áàéòîâ êîìàíäû äîëæíà âûïîëíÿòüñÿ òîëüêî ïîñëå ïðîâåðêè íåçàíÿòîñòè âõîäíîãî ðåãèñòðà êîíòðîëëåðà êëàâèàòóðû.
			if ((inp(0x64) & 0x02) == 0x00) //÷èòàåì ñîñòîÿíèå ïîðòà, ñâîáîäåí(0 â 1 áèòå) èëè çàíÿò(1 â 1 áèòå) ðåãèñòð 64h – ðåãèñòð ñîñòîÿíèÿ (ñòàòóñà)
			{
				break;
			}
		}
		if (!timeout)
		{
			printf("Timeout error.\n");
			disable();
			setvect(0x09, oldInt09);
			enable();
			exit(1);
		}
		//Äëÿ óïðàâëåíèÿ èíäèêàòîðàìè ÷åðåç 60h îòïðàâëÿåòñÿ êîä EDh. Çàòåì «ìàñêà», â ñîîòâåòñòâèè ñ êîòîðîé äîëæíû çàãîðåòüñÿ èíäèêàòîðû.

		outp(0x60, 0xED); //áóôåð ñâîáîäåí, ïèøåì óïðàâëÿþùèé áàéò
		delay(200);
	}
	timeout = 50000;
	repeat=1;
	while (repeat)
	{
		while (timeout-- > 0){
			if ((inp(0x64) & 0x02) == 0x00) //÷èòàåì ñîñòîÿíèå ïîðòà
			{
				break;
			}
		}
		if (!timeout)
		{
			printf("Timeout error.\n");
			disable();
			setvect(0x09, oldInt09);
			enable();
			exit(1);
		}
		outp(0x60, i); //áóôåð ñâîáîäåí, ïèøåì óïðàâëÿþùèé áàéò
		delay(200);
	}
}

int main()
{
	delay(200);
	disable();
	oldInt09 = getvect(0x09); //ïîëó÷àåì ñòàðûé îáðàáîò÷èê ïðåðûâàíèé
	setvect(0x09, newInt09);	//  
	enable();
	blinking = 1;
	for(int j=0; j<10;j++)
	{
		keyBlinking(0x04);
		delay(300);
		keyBlinking(0x00);
	}
	blinking = 0;
	
	while (!quitFlag);
	disable();
	setvect(0x09, oldInt09);
	enable();
	return 0;
}