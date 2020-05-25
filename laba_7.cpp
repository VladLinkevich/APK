#define _SCL_SECURE_NO_WARNINGS
#include <cstdio>
#include <windows.h>
#include <conio.h>
#include <algorithm>
#include <ctime>
#include <iostream>
#include <string>
#include <ctype.h>
#include <cstdlib>


const int kDelay = 100;

void Parent(char* path);													
void Child();																


int main(int argc, char* argv[])
{
	if (argc > 1) { Child(); }
	else { Parent(argv[0]); }
}

void Parent(char* path)
{

	HANDLE Work = CreateMutex(NULL, TRUE, "Work");					
	HANDLE Close = CreateMutex(NULL, TRUE, "Close");					

	std::string name = "COM1";

	STARTUPINFO si;												
	ZeroMemory(&si, sizeof(STARTUPINFO));
	si.cb = sizeof(STARTUPINFO);
	PROCESS_INFORMATION pi;										
	
	std::string message;


	

	HANDLE FileProjection = CreateFile(
		name.c_str(),														// Имя открываемого файла.
		GENERIC_READ | GENERIC_WRITE,										// Тип доступа к файлу.
		0,																	// Параметры совместного доступа.
		NULL,																// Атрибуты защиты файла.
		OPEN_EXISTING,														// Режим автосоздания.
		FILE_ATTRIBUTE_NORMAL,												// Асинхронный режим работы.
		NULL																// Описатель файла шаблона.
	);

	CreateProcess(
		path,																// Имя модуля
		(LPSTR)" Client",												    // Командная строка
		NULL,																// Дескриптор процесса
		NULL,																// Дескриптор потока
		FALSE,																// Установка описателей наследования
		NULL,																// Флаг создания процесса
		NULL,																// блок переменных окружения родительского процесса
		NULL,																// Текущий каталог родительскоого процесса
		&si,																// Указатель на структуру информаций о процессе
		&pi													
	);

	SetCommMask(FileProjection, EV_RXCHAR);									// EV_RXCHAR Символ принят и помещен в приемный буфер
	SetupComm(FileProjection, 1500, 1500);									// Инициализирует коммуникационные параметры для заданного устройства (Дескриптор, буфер ввода-вывода)

	COMMTIMEOUTS CommTimeOuts;												// Структура, характеризующая временные параметры последовательного порта.
	CommTimeOuts.ReadIntervalTimeout = 0xFFFFFFFF;							// Mаксимальное время для интервала между поступлением двух символов по линии связи.
	CommTimeOuts.ReadTotalTimeoutMultiplier = 0;							// Множитель, используемый, чтобы вычислить полный период времени простоя для операций чтения.
	CommTimeOuts.ReadTotalTimeoutConstant = kDelay;							// Константа, используемая, чтобы вычислить полный (максимальный) период времени простоя для операций чтения.
	CommTimeOuts.WriteTotalTimeoutMultiplier = 0;							// Множитель, используемый, чтобы вычислить полный период времени простоя для операций записи.
	CommTimeOuts.WriteTotalTimeoutConstant = kDelay;						// Константа, используемая, чтобы вычислить полный период времени простоя для операций записи.

	if (!SetCommTimeouts(FileProjection, &CommTimeOuts))					// устанавливает тайм-ауты порта.
	{
		CloseHandle(FileProjection);
		FileProjection = INVALID_HANDLE_VALUE;
		return;
	}

	DCB ComDCM;																// Структура, характеризующая основные параметры последовательного порта. 
	memset(&ComDCM, 0, sizeof(ComDCM));										// Выделение памяти под структуру.
	ComDCM.DCBlength = sizeof(DCB);											// Задает длину, в байтах, структуры.
	GetCommState(FileProjection, &ComDCM);									// Извлекает данные о текущих настройках управляющих сигналов для указанного устройства.
	ComDCM.BaudRate = DWORD(9600);											// Скорость передачи данных.
	ComDCM.ByteSize = 8;													// Определяет число информационных бит в передаваемых и принимаемых байтах.
	ComDCM.Parity = NOPARITY;												// Определяет выбор схемы контроля четности (Бит честности отсутствует).
	ComDCM.StopBits = ONESTOPBIT;											// Задает количество стоповых бит (Один бит). 
	ComDCM.fAbortOnError = TRUE;											// Задает игнорирование всех операций чтения/записи при возникновении ошибки.
	ComDCM.fDtrControl = DTR_CONTROL_DISABLE;								// Задает режим управления обменом для сигнала DTR.
	ComDCM.fRtsControl = RTS_CONTROL_DISABLE;								// Задает режим управления потоком для сигнала RTS.
	ComDCM.fBinary = TRUE;													// Включает двоичный режим обмена.
	ComDCM.fParity = FALSE;													// Включает режим контроля четности.
	ComDCM.fInX = FALSE;													// Задает использование XON/XOFF управления потоком при приеме.
	ComDCM.fOutX = FALSE;													// Задает использование XON/XOFF управления потоком при передаче.
	ComDCM.XonChar = 0;														// Задает символ XON используемый как для приема, так и для передачи.
	ComDCM.XoffChar = (unsigned char)0xFF;									// Задает символ XOFF используемый как для приема, так и для передачи.
	ComDCM.fErrorChar = FALSE;												// Задает символ, использующийся для замены символов с ошибочной четностью.
	ComDCM.fNull = FALSE;													// Указывает на необходимость замены символов с ошибкой четности на символ задаваемый полем ErrorChar.
	ComDCM.fOutxCtsFlow = FALSE;											// Включает режим слежения за сигналом CTS.
	ComDCM.fOutxDsrFlow = FALSE;											// Включает режим слежения за сигналом DSR.
	ComDCM.XonLim = 128;													// Задает минимальное число символов в приемном буфере перед посылкой символа XON.
	ComDCM.XoffLim = 128;													// Определяет максимальное количество байт в приемном буфере перед посылкой символа XOFF.

	if (!SetCommState(FileProjection, &ComDCM))
	{
		CloseHandle(FileProjection);
		FileProjection = INVALID_HANDLE_VALUE;
		return;
	}

	while (true)
	{
		DWORD NumberOfBytesWritten;
		std::cout << "Server: "; 
		std::getline(std::cin, message);


		if (message.empty())
		{																	
			ReleaseSemaphore(Close, 1, NULL);		
			ReleaseSemaphore(Work, 1, NULL);
			WaitForSingleObject(pi.hProcess, INFINITE);
			CloseHandle(pi.hThread);
			CloseHandle(pi.hProcess);
			CloseHandle(Close);
			CloseHandle(Work); 
			CloseHandle(FileProjection);
			break;
		}

									

		int size = message.size();
		WriteFile(FileProjection, &size, sizeof(size), &NumberOfBytesWritten, NULL);

		
		if (!WriteFile(FileProjection, message.c_str(), message.length(), &NumberOfBytesWritten, NULL))
		{	std::cout << "Error.";	}
		

		ReleaseMutex(Work);
		WaitForSingleObject(Work, INFINITE);						
	}

}



void Child()
{
	HANDLE hMyPipe;

	char* buffer;
	int size;
	
	std::string name = "COM2";

	bool successFlag;

	HANDLE Work = OpenMutex(MUTEX_ALL_ACCESS, FALSE, "Work");
	HANDLE Close = OpenMutex(MUTEX_ALL_ACCESS, FALSE, "Close");


	hMyPipe = CreateFile(name.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	while (true)
	{
		WaitForSingleObject(Work, INFINITE);
		
		
		successFlag = TRUE;
		DWORD NumberOfBytesRead;
	

		if (WaitForSingleObject(Close, kDelay) == WAIT_OBJECT_0) {


			CloseHandle(Work);
			CloseHandle(Close);
			CloseHandle(hMyPipe);

			break;
		}
		
		if (!ReadFile(hMyPipe, &size, sizeof(size), &NumberOfBytesRead, NULL)) { break; }

		buffer = new char[size];

		if (!ReadFile(hMyPipe, buffer, size, &NumberOfBytesRead, NULL)) { break; }
			
		std::cout << "Client got: ";

		for (int i = 0; i < size; i++)
		{
			std::cout << buffer[i];
			Sleep(100);
		}
		std::cout << std::endl;

		ReleaseMutex(Work);
	}

	
	return;
}