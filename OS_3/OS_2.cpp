/*
2. Дан файл, содержащий текст произвольной длины и ключ длиной k символов.
Выполнить шифрование этого текста по алгоритму, рассмотренному в задаче № 5 лабораторной работы № 1.
Для решения задачи реализовать три потока:
- поток чтения информации из файла в буфер;
- поток шифрования текста;
- поток вывода шифрованного текста в результирующий файл.
*/

#include <windows.h>
#include <stdio.h>
#include <iostream>
#include <locale>
#include <fstream>
#include <bitset>
using namespace std;

HANDLE hMutexRead,
hMutexEncrypt,
hMutexWrite,
hMutex;	  // Мьютекс доступа к переменной 

// разделяемая переменная
int checksum;
char* buffer;
int m;	// размер буфера
bool finished = false;
string key;
int blockSize = 0;

// Функция потока производителя
DWORD WINAPI readFile(LPVOID param)
{
	ifstream file("input.txt");

	while (true)
	{
		// Захватить мьютекс "информация обработана"	
		WaitForSingleObject(hMutexRead, INFINITE);

		// Обратиться к критической секции
		WaitForSingleObject(hMutex, INFINITE);

		// Читаем данные в буфер
		file.read(buffer, m);
		blockSize = (int)file.gcount();
		for (int i = 0; i < blockSize; i++) {
			cout << buffer[i];
		}

		if (blockSize == 0) {
			finished = true;
			ReleaseMutex(hMutex);
			ReleaseMutex(hMutexEncrypt);
			break;
		}

		// Освободить критическую секцию
		ReleaseMutex(hMutex);

		// Освободить мьютекс "информация обработана"
		ReleaseMutex(hMutexEncrypt);
	}
	file.close();
	return 0;
}

// Функция потока потребителя
DWORD WINAPI textEncrypt(LPVOID param)
{

	while (true)
	{
		// Захватить мьютекс "информация сгенерирована"	
		WaitForSingleObject(hMutexEncrypt, INFINITE);

		// Обратиться к критической секции
		WaitForSingleObject(hMutex, INFINITE);

		if (blockSize > 0) {
			for (int i = 0; i < blockSize / 2; i++) {
				swap(buffer[i], buffer[blockSize - i - 1]);
			}

			for (int i = 0; i < blockSize; i++) {
				buffer[i] = buffer[i] ^ key[i % key.length()];
			}

			ReleaseMutex(hMutex);
			ReleaseMutex(hMutexWrite);
		}
		if (finished) {
			ReleaseMutex(hMutex);
			ReleaseMutex(hMutexWrite);
			break;
		}

		ReleaseMutex(hMutex);
		ReleaseMutex(hMutexWrite);
	}
	return 0;
}


DWORD WINAPI textWrite(LPVOID param)
{
	ofstream file("output.txt", ios::binary);

	while (true)
	{
		// Захватить мьютекс "информация сгенерирована"	
		WaitForSingleObject(hMutexWrite, INFINITE);

		// Обратиться к критической секции
		WaitForSingleObject(hMutex, INFINITE);

		if (finished) {
			ReleaseMutex(hMutex);
			break;
		}

		if (blockSize > 0){
			for (int i = 0; i < blockSize; ++i) {
				file << bitset<8>((unsigned char)buffer[i]);
			}
			blockSize = 0;
		}

		// Освободить критическую секцию
		ReleaseMutex(hMutex);

		// Освободить мьютекс "информация обработана"
		ReleaseMutex(hMutexRead);
	}
	file.close();
	return 0;
}


int main()
{
	setlocale(LC_ALL, "rus");

	// Выделяем память для буфера
	buffer = new char[m];
	blockSize = 0;

	cout << "Введите ключ: ";
	cin >> key;

	m = key.length();                 // Размер буфера

	cout << "Текст: ";

	// Создаем мьютексы со значениями по умолчанию, все мьютексы созданы в состоянии "открыт"
	hMutexRead = CreateMutex(NULL, false, NULL);
	hMutexEncrypt = CreateMutex(NULL, false, NULL);
	hMutexWrite = CreateMutex(NULL, false, NULL);
	hMutex = CreateMutex(NULL, false, NULL);

	// Создаем потоки производителя и потребителя
	HANDLE hRead = CreateThread(NULL, 0, readFile,
		NULL, 0, NULL);
	HANDLE hEncrypt = CreateThread(NULL, 0, textEncrypt,
		NULL, 0, NULL);
	HANDLE hWrite = CreateThread(NULL, 0, textWrite, NULL, 0, NULL);


	HANDLE hThread[3];
	hThread[0] = hRead;
	hThread[1] = hEncrypt;
	hThread[2] = hWrite;

	// Дожидаемся завершения потоков
	WaitForMultipleObjects(3, hThread, true, INFINITE);

	cout << "\nЗашифрованный текст в output.txt" << endl;

	CloseHandle(hRead);
	CloseHandle(hEncrypt);
	CloseHandle(hWrite);

	return 0;
}
