/*
1. Дан файл содержащий текст произвольной длины. 
Найти контрольную сумму текста, как сумму кодов символов по модулю 256. 
Реализовать два потока: первый для чтения содержимого файла буфер длиной m символов, второй – для расчета контрольной суммы.
*/

#include <windows.h>
#include <stdio.h>
#include <iostream>
#include <locale>
#include <fstream>
using namespace std;

HANDLE hMutexReady,	  // Мьютекс "информация готова"
hMutexHandled,// Мьютекс "информация обработана"
hMutex;	  // Мьютекс доступа к переменной 

// разделяемая переменная
int checksum; 
char* buffer;
int m;	// размер буфера
int bytes_read;
bool finished = false;

// Функция потока производителя
DWORD WINAPI producer(LPVOID param)
{
	ifstream file("test.txt");

	while (true)
	{
		// Захватить мьютекс "информация обработана"	
		WaitForSingleObject(hMutexHandled, INFINITE);

		// Обратиться к критической секции
		WaitForSingleObject(hMutex, INFINITE);

		// Читаем данные в буфер
		file.read(buffer, sizeof(buffer));
		bytes_read = file.gcount();

		if (bytes_read == 0) {
			finished = true;
			ReleaseMutex(hMutex);
			ReleaseMutex(hMutexReady);
			break;
		}

		// Освободить критическую секцию
		ReleaseMutex(hMutex);

		// Освободить мьютекс "информация обработана"
		ReleaseMutex(hMutexReady);
	}
	file.close();
	return 0;
}

// Функция потока потребителя
DWORD WINAPI consumer(LPVOID param)
{
	
	while (true)
	{
		// Захватить мьютекс "информация сгенерирована"	
		WaitForSingleObject(hMutexReady, INFINITE);

		// Обратиться к критической секции
		WaitForSingleObject(hMutex, INFINITE);

		if (finished) {
			ReleaseMutex(hMutex);
			ReleaseMutex(hMutexHandled);
			break;
		}

		for (int i = 0; i < bytes_read; i++) {
			cout << buffer[i] << " " << (int)(buffer[i]) << endl;
			checksum = checksum + (int)(buffer[i]);
		}

		// Освободить критическую секцию
		ReleaseMutex(hMutex);

		// Освободить мьютекс "информация обработана"
		ReleaseMutex(hMutexHandled);
	}

	return 0;
}

int main()
{
	setlocale(LC_ALL, "rus");

	m = 10;                 // Размер буфера

	// Выделяем память для буфера
	buffer = new char[m];
	bytes_read = 0;
	checksum = 0;

	// Создаем мьютексы со значениями по умолчанию, все мьютексы созданы в состоянии "открыт"
	hMutexReady = CreateMutex(NULL, false, NULL);
	hMutexHandled = CreateMutex(NULL, false, NULL);
	hMutex = CreateMutex(NULL, false, NULL);

	// Создаем потоки производителя и потребителя
	HANDLE hProducer = CreateThread(NULL, 0, producer,
		NULL, 0, NULL);
	HANDLE hConsumer = CreateThread(NULL, 0, consumer,
		NULL, 0, NULL);

	HANDLE hThread[2];
	hThread[0] = hProducer;
	hThread[1] = hConsumer;

	// Дожидаемся завершения обоих потоков
	WaitForMultipleObjects(2, hThread, true, INFINITE);

	cout << "Контрольная сумма файла: " << checksum % 256;

	CloseHandle(hProducer);
	CloseHandle(hConsumer);

	return 0;
}
