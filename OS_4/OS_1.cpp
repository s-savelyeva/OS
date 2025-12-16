/*
1. На некотором предприятии имеется n цехов, которые независимо друг от друга формируют заявки на приобретение материалов.
Все заявки поступают в единый обрабатывающий центр, который должен сформировать единую заявку в виде массива записей,
содержащих информацию о наименовании и общем количестве этого материала. 
Каждый цех может сформировать не более чем k заявок. Напишите программу для формирования общей заявки при указанных условиях.
*/

#include <windows.h>
#include <iostream>
#include <locale.h>
#include <ctime>
#include <random>
#include <map>
using namespace std;

const int n = 3;  // количество цехов
const int k = 4;  // максимальное количество заявок от одного цеха
const int buffer_size = 10;  // размер буфера

// Описание структуры буфера и самого буфера
struct Material
{
	string name;
	int amount;
} buffer[buffer_size];

// Позиции в буфере
int cur_pos = 0;
int process_pos = 0;

int completed_writers = 0;

// Значения для итоговых переменных
map<string, int> total_order;

HANDLE hSemFull,
hSemEmpty,
hMutex;

string materials[] = { "Сталь", "Алюминий", "Медь", "Пластик", "Дерево" };

// Функция писателя
DWORD WINAPI writer(LPVOID param)
{
	int workshop_id = GetCurrentThreadId();
	srand(time(NULL) + workshop_id);
	int requests_count = 1 + rand() % k;  // от 1 до k заявок

	for (int i = 0; i < requests_count; i++)
	{
		// Дождаться освобождения ячеек
		WaitForSingleObject(hSemEmpty, INFINITE);
		// Захватить критический ресурс и сформировать новую операцию
		WaitForSingleObject(hMutex, INFINITE);

		buffer[cur_pos].name = materials[rand() % 5];
		buffer[cur_pos].amount = 10 + rand() % 100;

		cout << "Цех " << workshop_id << " добавил: "
			<< buffer[cur_pos].name << " - "
			<< buffer[cur_pos].amount << " ед." << endl;

		cur_pos = (cur_pos + 1) % buffer_size;

		// Освободить критический ресурс 
		ReleaseMutex(hMutex);
		// Увеличить число занятых ячеек
		ReleaseSemaphore(hSemFull, 1, NULL);
		
	}
	// Увеличиваем счетчик завершенных писателей
	WaitForSingleObject(hMutex, INFINITE);
	completed_writers++;
	ReleaseMutex(hMutex);
	return 0;
}

// Функция читателя
DWORD WINAPI reader(LPVOID param)
{
	
	while (true)
	{
		// Дождаться появления заполненных ячеек
		DWORD result = WaitForSingleObject(hSemFull, 1000);

		if (result == WAIT_TIMEOUT)
		{
			// Проверяем, все ли писатели завершили работу и буфер пуст
			WaitForSingleObject(hMutex, INFINITE);
			bool all_done = (completed_writers == n) && (process_pos == cur_pos);
			ReleaseMutex(hMutex);

			if (all_done) {
				break;  // все завершено, выходим
			}
			else {
				continue;  // продолжаем ждать
			}
		}

		// Захватить критический ресурс и обработать операцию
		WaitForSingleObject(hMutex, INFINITE);

		Material material = buffer[process_pos];
		total_order[material.name] += material.amount;

		process_pos = (process_pos + 1) % buffer_size;

		// Освободить критический ресурс
		ReleaseMutex(hMutex);
		// Увеличить число пустых ячеек
		ReleaseSemaphore(hSemEmpty, 1, NULL);
		cout << "Обработана заявка: " << material.name
			<< " - " << material.amount << " ед." << endl;

	}
	return 0;
}

int main()
{
	setlocale(LC_ALL, "Rus");
	// Массивы дескрипторов потокеов
	HANDLE	hReaders,
		hWriters[n];

	cur_pos = 0;
	// Создание семафоров
	// семафор занятых ячеек создаем закрытым
	hSemFull = CreateSemaphore(NULL, 0, buffer_size, NULL);
	// семафор пустых ячеек создаем открытым
	hSemEmpty = CreateSemaphore(NULL, buffer_size, buffer_size, NULL);
	hMutex = CreateMutex(NULL, false, NULL);

	// Запуск потоков
	for (int i = 0; i < n; i++)
	{
		hWriters[i] = CreateThread(NULL, 0, writer,
			NULL, 0, NULL);
	}

	hReaders = CreateThread(NULL, 0, reader,
		NULL, 0, NULL);

	// Необходимо дождаться завершения всех потоков
	WaitForMultipleObjects(n, hWriters, true, INFINITE);
	WaitForSingleObject(hReaders, INFINITE);

	cout << "Заявка:\n";
	for (const auto& item : total_order) {
		cout << "  " << item.first << ": " << item.second << " ед." << endl;
	}

	for (int i = 0; i < n; i++)
		CloseHandle(hWriters[i]);

	CloseHandle(hReaders);

	CloseHandle(hSemFull);
	CloseHandle(hSemEmpty);
	CloseHandle(hMutex);

	return 0;
}
