#include <iostream>
#include <locale.h>
#include <windows.h>
using namespace std;


const int m = 10, n = 20;
float mtx[m][n],		// матрица
col_sums[n];	// массив сумм

int col_numbers[n], // номера столбцов
row_numbers[m]; //номера строк

// Функция потока
DWORD WINAPI col_sum(LPVOID param)
{
	// Получаем значение параметра 
//- номер столбца
	int* pcol_num = (int*)param;
	int col_num = *pcol_num;

	// Находим искомую сумму
	col_sums[col_num] = 0;
	for (int i = 0; i < m; i++)
		col_sums[col_num] += (mtx[i][col_num]);

	return 0;
}

DWORD WINAPI fill_mtx(LPVOID param) {

	int* prow_num = (int*)param;
	int row_num = *prow_num;

	//cout << "Заполнение строки №" << row_num << endl;

	srand((int)time(0) + row_num * 100);
	for (int i = 0; i < n; i++) {
		mtx[row_num][i] = (float)(rand());
	}
	return 0;
}

int main()
{
	setlocale(LC_ALL, "Russian");

	// Описание переменных для работы с потоками
	// -- массив из n указателей потоков
	HANDLE colThread[n];
	HANDLE rowThread[m];
	// -- массив из n идентификаторов потоков
	DWORD colThreadID[n];
	DWORD rowThreadID[m];

	// Заполнение массивов исходными значениями
	for (int i = 0; i < m; i++)
	{
		row_numbers[i] = i;
	}

	for (int i = 0; i < m; i++) {
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
	WaitForMultipleObjects(m, rowThread,
		true, INFINITE);
	// Закрытие потоков
	for (int i = 0; i < m; i++)
		CloseHandle(rowThread[i]);

	cout << "Массив:" << endl;
	for (int i = 0; i < m; i++) {
		for (int j = 0; j < n; j++) {
			printf("%6.0f", mtx[i][j]);
		}
		cout << endl;
	}

	// Заполнение массивов исходными значениями
	for (int i = 0; i < n; i++)
	{
		col_sums[i] = 0;
		col_numbers[i] = i;
	}

	//Запуск потоков
	for (int i = 0; i < n; i++)
	{
		colThread[i] = CreateThread(
			// атрибуты безопасности по умолчанию
			NULL,
			// размер стека по умолчанию
			0,
			// имя функции				  
			col_sum,
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
	WaitForMultipleObjects(n, colThread,
		true, INFINITE);
	

	// Находим номер столбца с минимальной суммой
	int num_min = 0;
	float min = col_sums[0];

	for (int i = 1; i < n; i++)
	{
		if (min > col_sums[i])
		{
			min = col_sums[i];
			num_min = i;
		}
	}

	// Вывод результата
	std::cout << "Искомый столбец № "
		<< num_min << '\n';

	// Закрытие потоков
	for (int i = 0; i < n; i++)
		CloseHandle(colThread[i]);

	return 0;
}
