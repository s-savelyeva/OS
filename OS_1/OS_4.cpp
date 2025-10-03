#include <iostream>
#include <locale.h>
#include <windows.h>
#include <time.h>
using namespace std;


const int n = 4, m = 4;
float mtx[n][m];		// матрица
float det_mas[m];
int row_numbers[n]; //номера столбцов
int col_numbers[m];

float det3(float m[3][3]) {
	return
		m[0][0] * m[1][1] * m[2][2] +
		m[0][1] * m[1][2] * m[2][0] +
		m[0][2] * m[1][0] * m[2][1] -
		m[0][2] * m[1][1] * m[2][0] -
		m[0][0] * m[1][2] * m[2][1] -
		m[0][1] * m[1][0] * m[2][2];
}

// Функция потока
DWORD WINAPI find_det(LPVOID param)
{
	// Получаем значение параметра 
//- номер столбца
	int* pcol_num = (int*)param;
	int col_num = *pcol_num;

	int row = 0, col = 0;
	float minor[3][3];

	for (int i = 1; i < n; i++) {
		col = 0;
		for (int j = 0; j < m; j++) {
			if (j == col_num) continue;
			minor[row][col] = mtx[row][col];
			col++;
		}
		row++;
	}

	float det_minor = det3(minor);
	float sign = (col_num % 2 == 0) ? 1.0f : -1.0f;

	det_mas[col_num] = sign * mtx[0][col_num] * det_minor;

	return 0;
}


int main()
{
	setlocale(LC_ALL, "Russian");
	srand(time(0));

	// Описание переменных для работы с потоками
	// -- массив из n указателей потоков
	HANDLE colThread[m];
	// -- массив из n идентификаторов потоков
	DWORD colThreadID[m];

	for (int i = 0; i < m; i++) {
		col_numbers[i] = i;
	}

	for (int i = 0; i < n; i++) {
		for (int j = 0; j < m; j++) {
			mtx[i][j] = (float)(rand());
		}
	}

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
			find_det,
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

	float det = 0;
	for (int i = 0; i < n; i++) det += det_mas[i];

	cout << "Определитель матрицы = " << det;

	// Закрытие потоков
	for (int i = 0; i < n; i++)
		CloseHandle(colThread[i]);

	return 0;
}
