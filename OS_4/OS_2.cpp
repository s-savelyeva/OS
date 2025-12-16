/*
2. В текстовом файле находится текст произвольной длины на английском языке. Необходимо выполнить шифрование этого текста по следующему алгоритму:
- все буквы в тексте преобразуются в заглавные;
- каждая буква заменяется на следующую по алфавиту, т.е. А заменяется на В, В – на С, и т.д. Буква Z должна быть заменена на A.
Зашифрованный текст должен быть помещен в новый текстовый файл.
Выборку текста из файла выполняет один поток, который считывает символы последовательно в буфер, длиной m символов.
Каждый новый символ записывается в первую свободную ячейку с конца буфера.
Шифрование текста осуществляют n потоков-шифровщиков, которые модифицируют буквы в буфере по указанным правилам.
Запись шифрованного текста выполняет третий поток, который вычитывает символы из буфера от начала к концу и освобождает ячейки для записи новых символов.
*/

#include <windows.h>
#include <iostream>
#include <locale.h>
#include <ctime>
#include <fstream>
#include <string>
using namespace std;

const int n = 3;  // количество цехов
const int buffer_size = 10;  // размер буфера

// Позиции в буфере

bool reader_done = false;

HANDLE hSemFull,
hSemEmpty,
hMutex;
HANDLE hSemEncrypted; // семафор зашифрованных ячеек

// Структура буфера
struct Buffer {
	char data[buffer_size];
	bool empty[buffer_size];  // флаги пустых ячеек
	int read_pos;             // позиция для чтения
	int encrypt_pos;
	int write_pos;            // позиция для записи
	int count;                // количество символов в буфере
};

Buffer buffer;

// Функция для шифрования символа
char encrypt_char(char c) {
	if (c >= 'A' && c <= 'Z') {
		if (c == 'Z') return 'A';
		return c + 1;
	}
	return c;  // не буквы не шифруем
}

// Функция читателя
DWORD WINAPI reader(LPVOID param)
{
	ifstream file("input.txt");

	char c;

	while (file.get(c))
	{

		// Дождаться освобождения ячеек
		WaitForSingleObject(hSemEmpty, INFINITE);
		// Захватить критический ресурс и сформировать новую операцию
		WaitForSingleObject(hMutex, INFINITE);
		c = toupper(c);

		buffer.data[buffer.write_pos] = c;
		buffer.empty[buffer.write_pos] = false;
		buffer.write_pos = (buffer.write_pos + 1) % buffer_size;
		buffer.count++;

		cout << "Читатель записал: '" << c << "' в позицию "
			<< (buffer.write_pos - 1 + buffer_size) % buffer_size << endl;

		// Освободить критический ресурс 
		ReleaseMutex(hMutex);
		// Увеличить число занятых ячеек
		ReleaseSemaphore(hSemFull, 1, NULL);

	}
	file.close();
	reader_done = true;
	return 0;
}

DWORD WINAPI encryptor(LPVOID param) {

	int id = GetCurrentThreadId();

	while (true) {

		WaitForSingleObject(hSemFull, 100);

		// Захватить критический ресурс и обработать операцию
		WaitForSingleObject(hMutex, INFINITE);

		if (reader_done && buffer.count == 0) {
			ReleaseSemaphore(hSemEncrypted, 1, NULL);
			// Освободить критический ресурс
			ReleaseMutex(hMutex);
			break;
		}

		bool found = false;

		for (int i = 0; i < buffer_size; i++) {
			int pos = buffer.encrypt_pos;

			if (!buffer.empty[pos]) {
				char original = buffer.data[pos];
				buffer.data[pos] = encrypt_char(original);
				buffer.encrypt_pos = (buffer.encrypt_pos + 1) % buffer_size;

				found = true;
				break;
			}
		}

		ReleaseSemaphore(hSemEncrypted, 1, NULL);
		// Освободить критический ресурс
		ReleaseMutex(hMutex);
		

	}
	return 0;
}


// Функция писателя
DWORD WINAPI writer(LPVOID param)
{
	ofstream file("output.txt");

	while (true)
	{

		WaitForSingleObject(hSemEncrypted, 2000);

		// Захватить критический ресурс и обработать операцию
		WaitForSingleObject(hMutex, INFINITE);

		if (reader_done && buffer.count == 0) {
			ReleaseMutex(hMutex);
			ReleaseSemaphore(hSemEmpty, 1, NULL);
			break;
		}

		for (int i = 0; i < buffer_size; i++) {

			int pos = (buffer.read_pos + i) % buffer_size;

			if (!buffer.empty[pos]) {

				file << buffer.data[pos];

				cout << "Писатель записал: '" << buffer.data[pos] << "' из позиции " << buffer.read_pos << endl;

				// Освобождаем ячейку
				buffer.empty[pos] = true;
				if (pos == buffer.read_pos) {
					buffer.read_pos = (buffer.read_pos + 1) % buffer_size;
				}
				buffer.count--;
				break;
			}

		}
		// Освободить критический ресурс
		ReleaseMutex(hMutex);
		// Увеличить число пустых ячеек
		ReleaseSemaphore(hSemEmpty, 1, NULL);

	}
	file.close();
	return 0;
}

int main()
{
	setlocale(LC_ALL, "Rus");
	// Массивы дескрипторов потокеов
	HANDLE	hReader, hWriter, hEncryptors[n];

	buffer.read_pos = 0;
	buffer.write_pos = 0;
	buffer.count = 0;
	for (int i = 0; i < buffer_size; i++) {
		buffer.empty[i] = true;
	}

	// Создание семафоров
	// семафор занятых ячеек создаем закрытым
	hSemFull = CreateSemaphore(NULL, 0, buffer_size, NULL);
	// семафор пустых ячеек создаем открытым
	hSemEmpty = CreateSemaphore(NULL, buffer_size, buffer_size, NULL);
	hSemEncrypted = CreateSemaphore(NULL, 0, buffer_size, NULL);
	hMutex = CreateMutex(NULL, false, NULL);

	// Запуск потоков
	for (int i = 0; i < n; i++)
	{
		hEncryptors[i] = CreateThread(NULL, 0, encryptor,
			NULL, 0, NULL);
	}

	hReader = CreateThread(NULL, 0, reader,
		NULL, 0, NULL);
	hWriter = CreateThread(NULL, 0, writer,
		NULL, 0, NULL);

	// Необходимо дождаться завершения всех потоков
	WaitForMultipleObjects(n, hEncryptors, true, INFINITE);
	WaitForSingleObject(hReader, INFINITE);
	WaitForSingleObject(hWriter, INFINITE);

	for (int i = 0; i < n; i++)
		CloseHandle(hEncryptors[i]);

	CloseHandle(hReader);
	CloseHandle(hWriter);

	CloseHandle(hSemFull);
	CloseHandle(hSemEmpty);
	CloseHandle(hSemEncrypted);
	CloseHandle(hMutex);

	return 0;
}
