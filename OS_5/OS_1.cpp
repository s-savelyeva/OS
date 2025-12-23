/*
1. Написать программу выполняющую сортировку массива из n вещественных чисел методом обмена. 
Каждый проход сортировки запустить в отдельном потоке. 
Поток следующего прохода активизировать только тогда, когда предыдущий проход обработал половину массива.
*/

#include <windows.h>
#include <iostream>
#include <locale.h>
#include <iomanip>
using namespace std;

HANDLE hEventHalfSorted;     // событие - половина массива отсортирована
HANDLE hEventPassComplete;   // событие - проход завершен
HANDLE hMutex;               // мьютекс для защиты доступа к массиву
HANDLE hStartEvent;          // событие для запуска первого потока

double* arr = NULL;          // массив для сортировки
int n;                       // размер массива

// функция производителя
DWORD WINAPI sort(LPVOID param)
{

	int passNum = *(int*)param;

	if (passNum == 0) {
		// Первый поток ждет сигнала начала
		WaitForSingleObject(hStartEvent, INFINITE);
	}
	else {
		// Остальные потоки ждут, пока предыдущий проход обработает половину массива
		WaitForSingleObject(hEventHalfSorted, INFINITE);
		ResetEvent(hEventHalfSorted);
	}

	bool swapped;
	int halfPoint = (n - passNum) / 2 + passNum; // Точка половины для текущего прохода

	do {
		swapped = false;

		WaitForSingleObject(hMutex, INFINITE);

		for (int i = 0; i < n - 1 - passNum; i++) {
			if (arr[i] > arr[i + 1]) {
				swap(arr[i], arr[i + 1]);
				swapped = true;
			}

			// Если достигли половины неотсортированной части - сигнализируем
			if (i == halfPoint - 1) {
				SetEvent(hEventHalfSorted);
				ResetEvent(hEventPassComplete);
			}
		}

		ReleaseMutex(hMutex);

	} while (swapped);

	return 0;
}


int main()
{
	setlocale(LC_ALL, "Rus");
	srand(int(time(NULL)));

	// создание событий - оба в ручном режиме
	// событие ввода создаем в состоянии "не произошло"
	hEventHalfSorted = CreateEvent(NULL, TRUE, TRUE, NULL);   // ручной сброс
	hEventPassComplete = CreateEvent(NULL, TRUE, TRUE, NULL); // ручной сброс
	hStartEvent = CreateEvent(NULL, TRUE, FALSE, NULL);        // ручной сброс
	hMutex = CreateMutex(NULL, FALSE, NULL);

	cout << "Введите размер массива: ";
	cin >> n;

	// Выделение памяти для массива
	arr = new double[n];

	// Заполнение массива случайными числами
	cout << "Исходный массив: ";
	for (int i = 0; i < n; i++) {
		arr[i] = rand() % 100;
		cout << fixed << setprecision(2) << arr[i] << " ";
	}
	cout << endl;

	// Создание потоков для каждого прохода сортировки
	// Максимум нужно n-1 проходов
	HANDLE* hThreads = new HANDLE[n - 1];
	DWORD* threadIDs = new DWORD[n - 1];

	for (int i = 0; i < n - 1; i++) {
		int* passNum = new int(i);
		hThreads[i] = CreateThread(NULL, 0, sort, passNum, 0, &threadIDs[i]);

		if (!hThreads[i]) {
			cout << "Ошибка создания потока " << i + 1 << endl;
		}
	}

	cout << "Начало сортировки" << endl;
	SetEvent(hStartEvent);

	// Ждем завершения всех потоков
	WaitForMultipleObjects(n - 1, hThreads, TRUE, INFINITE);

	cout << "Отсортированный массив: ";
	for (int i = 0; i < n; i++) {
		cout << fixed << setprecision(2) << arr[i] << " ";
	}
	cout << endl;

	// Очистка ресурсов
	for (int i = 0; i < n - 1; i++) {
		CloseHandle(hThreads[i]);
	}

	CloseHandle(hEventHalfSorted);
	CloseHandle(hEventPassComplete);
	CloseHandle(hStartEvent);
	CloseHandle(hMutex);

	delete[] hThreads;
	delete[] threadIDs;
	delete[] arr;
	return 0;
}
