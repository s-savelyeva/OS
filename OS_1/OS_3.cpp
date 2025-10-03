#include <iostream>
#include <locale.h>
#include <windows.h>
#include <time.h>
using namespace std;


const int n = 10, m = 20;
float mtx[n][m];		// матрица

int col_numbers[m]; //номера столбцов
int row_numbers[n];

// Функция потока
DWORD WINAPI bubble_sort(LPVOID param)
{
	// Получаем значение параметра 
//- номер столбца
	int* pcol_num = (int*)param;
	int col_num = *pcol_num;

	for (int i = 0; i < n; i++) {
		for (int j = 0; j < n - 1; j++) {
			if (mtx[j][col_num] > mtx[j + 1][col_num]) {
				float temp = mtx[j][col_num];
				mtx[j][col_num] = mtx[j + 1][col_num];
				mtx[j + 1][col_num] = temp;
			}
		}
	}
	
	return 0;
}

DWORD WINAPI fill_mtx(LPVOID param) {

	int* prow_num = (int*)param;
	int row_num = *prow_num;

	//cout << "Заполнение строки №" << row_num << endl;
	srand((int)time(0) + row_num * 100);
	for (int i = 0; i < m; i++) {
		mtx[row_num][i] = (float)(rand());
	}
	return 0;
}

int main()
{
	setlocale(LC_ALL, "Russian");

	// Описание переменных для работы с потоками
	// -- массив из n указателей потоков
	HANDLE colThread[m];
	HANDLE rowThread[n];
	// -- массив из n идентификаторов потоков
	DWORD colThreadID[m];
	DWORD rowThreadID[n];

	for (int i = 0; i < m; i++) {
		col_numbers[i] = i;
	}

	for (int i = 0; i < n; i++) {
		row_numbers[i] = i;
	}

	for (int i = 0; i < n; i++) {
		rowThread[i] = CreateThread(
			// атрибуты безопасности по умолчанию
			NULL,
			// размер стека по умолчанию
			0,
			// имя функции				  
			fill_mtx,
			// указатель на параметры		  
			&(row_numbers[i]),
			// флаг создания = 0
			0,
			// адрес переменной для идентификатора			
			&(rowThreadID[i]));

		// Проверям - создан ли поток
		if (rowThread[i] == NULL)
		{
			std::cout << "Поток №s " << i
				<< "не был создан\n"
				<< "Ошибка "
				<< GetLastError();
		}
	}

	// Ожидаем завершения потоков
	WaitForMultipleObjects(n, rowThread,
		true, INFINITE);

	for (int i = 0; i < n; i++)
		CloseHandle(rowThread[i]);

	cout << "Массив:" << endl;
	for (int i = 0; i < n; i++) {
		for (int j = 0; j < m; j++) {
			printf("%6.0f", mtx[i][j]);
		}
		cout << endl;
	}

	//Запуск потоков
	for (int i = 0; i < m; i++)
	{
		colThread[i] = CreateThread(
			// атрибуты безопасности по умолчанию
			NULL,
			// размер стека по умолчанию
			0,
			// имя функции				  
			bubble_sort,
			// указатель на параметры		  
			&(col_numbers[i]),
			// флаг создания = 0
			0,
			// адрес переменной для идентификатора			
			&(colThreadID[i]));

		// Проверям - создан ли поток
		if (colThread[i] == NULL)
		{
			std::cout << "Поток №s " << i
				<< "не был создан\n"
				<< "Ошибка "
				<< GetLastError();
		}
	}

	// Ожидаем завершения потоков
	WaitForMultipleObjects(m, colThread,
		true, INFINITE);

	cout << "Отсортированный массив:" << endl;
	for (int i = 0; i < n; i++) {
		for (int j = 0; j < m; j++) {
			printf("%6.0f", mtx[i][j]);
		}
		cout << endl;
	}

	// Закрытие потоков
	for (int i = 0; i < n; i++)
		CloseHandle(colThread[i]);

	return 0;
}
