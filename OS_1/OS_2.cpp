#include <iostream>
#include <locale.h>
#include <windows.h>
using namespace std;

const int n = 10, m = 20;
float mtx[n][m],		// матрица
row_avgs[n];	// массив сумм

int row_numbers[n]; //номера строк

// Функция потока
DWORD WINAPI row_avg(LPVOID param)
{
	// Получаем значение параметра 
//- номер столбца
	int* prow_num = (int*)param;
	int row_num = *prow_num;

	// Находим искомую сумму
	row_avgs[row_num] = 0;
	for (int i = 0; i < m; i++)
		row_avgs[row_num] += (mtx[row_num][i]);
	row_avgs[row_num] = row_avgs[row_num] / m;

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
	HANDLE rowThread[n];
	// -- массив из n идентификаторов потоков
	DWORD rowThreadID[n];

	// Заполнение массивов исходными значениями
	for (int i = 0; i < n; i++)
	{
		row_numbers[i] = i;
		row_avgs[i] = 0;
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

	cout << "Массив:" << endl;
	for (int i = 0; i < n; i++) {
		for (int j = 0; j < m; j++) {
			printf("%6.0f", mtx[i][j]);
		}
		cout << endl;
	}

	//Запуск потоков
	for (int i = 0; i < n; i++)
	{
		rowThread[i] = CreateThread(
			// атрибуты безопасности по умолчанию
			NULL,
			// размер стека по умолчанию
			0,
			// имя функции				  
			row_avg,
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


	// Находим номер строки с максимальным средним значением
	int num_max = 0;
	float max = row_avgs[0];

	for (int i = 1; i < n; i++)
	{
		if (row_avgs[i] > max)
		{
			max = row_avgs[i];
			num_max = i;
		}
	}

	// Вывод результата
	std::cout << "Искомая строка № "
		<< num_max << '\n';

	// Закрытие потоков
	for (int i = 0; i < n; i++)
		CloseHandle(rowThread[i]);

	return 0;
}
