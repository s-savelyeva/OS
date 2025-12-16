/*
4. Напишите программу управления пулом печати. В системе, имеется пул печати объемом n байт, поток управления пулом и m потоков, осуществляющих печать.
Поток, получивший доступ к пулу может записать в него либо весь текст, предназначенный для печати, либо часть текста, если места в пуле недостаточно.
Если места в пуле недостаточно, то поток должен быть приостановлен до тех пор, пока поток управления пулом не освободит место в пуле. 
Поток, начавший вывод информации в пул не может быть прерван другими потоками вывода текста до тех пор, пока не осуществит вывод всего текста. 
Запись текста всегда осуществляется в конец пула.
Поток управления пулом может получить доступ к пулу только тогда, когда в пуле имеются заполненные ячейки. Чтение из пула осуществляется из начала. 
При этом остальное содержимое пула сдвигается к началу – выведенный на печать текст удаляется.
Для имитации работы пула используйте потоки, читающие текстовые файлы, и поток управления, выполняющий запись текста в файл.
*/

#include <windows.h>
#include <iostream>
#include <locale.h>
#include <fstream>
#include <string>
using namespace std;

const int m = 3;                   //число потоков (писателей)
const int poolSize = 10;            //размер пула
char* pool;              //пул
int poolUsed = 0;        //кол-во занятых байт
bool finished = false;

HANDLE hMutex, hSemFull, hSemEmpty;

struct ThreadParams {
	string filename;
};
ThreadParams* params;


// Функция читателя
DWORD WINAPI reader(LPVOID param)
{
	ThreadParams* params = (ThreadParams*)param;
	ifstream file(params->filename);

	while (true)
	{

		// Захватить критический ресурс и сформировать новую операцию
		WaitForSingleObject(hMutex, INFINITE);

		int space = poolSize - poolUsed;   //доступное место
		if (space == 0) {
			ReleaseMutex(hMutex);
			WaitForSingleObject(hSemEmpty, INFINITE);
			continue;
		}
		file.read(&pool[poolUsed], space);	//читаем с позиции poolUsed!
		int readed = (int)file.gcount();

		if (readed == 0) {
			finished = true;
			ReleaseMutex(hMutex);
			break;
		}
		poolUsed += readed;

		cout << "Прочитано " << readed << " байт из файла '" << (*params).filename << "'" << endl;

		// Освободить критический ресурс 
		ReleaseMutex(hMutex);
		// Увеличить число занятых ячеек
		ReleaseSemaphore(hSemFull, readed, NULL);

	}
	file.close();
	return 0;
}


// Функция писателя
DWORD WINAPI writer(LPVOID param)
{
	ofstream file("output.txt");

	while (true)
	{

		WaitForSingleObject(hSemFull, INFINITE);

		// Захватить критический ресурс и обработать операцию
		WaitForSingleObject(hMutex, INFINITE);

		if (poolUsed == 0 && finished) {
			ReleaseMutex(hMutex);
			break;
		}

		int toRead = min(poolUsed, poolSize);  // Берем минимум из двух значений

		if (toRead > 0) {
			for (int i = 0; i < toRead; i++) {
				file.put(pool[i]);
			}
			for (int i = 0; i < poolUsed - toRead; i++) {
				pool[i] = pool[toRead + i];
			}
			poolUsed -= toRead;

			cout << "Записано " << toRead << " байт в файл 'output.txt'" << endl;

			ReleaseSemaphore(hSemEmpty, toRead, NULL);
		}
		ReleaseMutex(hMutex);
		if (toRead == 0 && finished) {
			break;
		}
	}
	file.close();
	return 0;
}

int main()
{
	setlocale(LC_ALL, "Rus");
	// Массивы дескрипторов потокеов
	HANDLE hReader[m], hWriter;

	const string inputFiles[] = {"input1.txt", "input2.txt", "input3.txt"};
	pool = new char[poolSize];
	params = new ThreadParams[m];

	// Создание семафоров
	// семафор занятых ячеек создаем закрытым
	hSemFull = CreateSemaphore(NULL, 0, poolSize, NULL);
	// семафор пустых ячеек создаем открытым
	hSemEmpty = CreateSemaphore(NULL, poolSize, poolSize, NULL);
	hMutex = CreateMutex(NULL, false, NULL);

	for (int i = 0; i < m; i++)
	{
		params[i].filename = inputFiles[i];
	}

	// Запуск потоков
	for (int i = 0; i < m; i++)
	{
		hReader[i] = CreateThread(NULL, 0, reader,
			&params[i], 0, NULL);
	}

	hWriter = CreateThread(NULL, 0, writer,
		NULL, 0, NULL);

	// Необходимо дождаться завершения всех потоков
	WaitForMultipleObjects(m, hReader, true, INFINITE);
	WaitForSingleObject(hWriter, INFINITE);

	for (int i = 0; i < m; i++)
		CloseHandle(hReader[i]);

	CloseHandle(hWriter);
	CloseHandle(hSemFull);
	CloseHandle(hSemEmpty);
	CloseHandle(hMutex);

	return 0;
}
