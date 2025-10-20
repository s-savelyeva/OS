#include <windows.h>
#include <iostream>
#include <ctime>
#include <locale>
#include <iomanip>

using namespace std;

const int m = 3, n = 3;
float mtx[n][m];		// матрица
float sum = 0;
int row_numbers[n]; //номера строк
bool row_ready[n];      // флаги готовности строк
CRITICAL_SECTION cs;

DWORD WINAPI fill_mtx(LPVOID param) {

    int* prow_num = (int*)param;
    int row_num = *prow_num;

    srand((int)time(0) + row_num * 100);
    for (int i = 0; i < n; i++) {
        mtx[row_num][i] = (float)(rand()%10);
    }
    row_ready[row_num] = true;
    return 0;
}

DWORD WINAPI find_sum(LPVOID param) {
    int* prow_num = (int*)param;
    int row_num = *prow_num;

    // Ждем, пока строка будет сформирована
    bool ready = false;
    while (!ready) {
        EnterCriticalSection(&cs);  // Добавляем синхронизацию для чтения флага
        ready = row_ready[row_num];
        LeaveCriticalSection(&cs);
        if (!ready) {
            Sleep(1); // небольшая пауза
        }
    }

    for (int i = 0; i < n; i++) {
        EnterCriticalSection(&cs);
        sum += mtx[row_num][i];
        LeaveCriticalSection(&cs);
    }
    return 0;
}

int main() {
    setlocale(LC_ALL, "rus");

    HANDLE fillThread[n];
    HANDLE calcThread[n];
    DWORD fillThreadID[n];
    DWORD calcThreadID[n];

    // Заполнение массивов исходными значениями
    for (int i = 0; i < m; i++)
    {
        row_numbers[i] = i;
        row_ready[i] = false;
    }

    InitializeCriticalSection(&cs);

    for (int i = 0; i < n; i++) {
        fillThread[i] = CreateThread(
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
            &(fillThreadID[i]));

        // Проверям - создан ли поток
        if (fillThread[i] == NULL)
        {
            std::cout << "Поток №s " << i
                << "не был создан\n"
                << "Ошибка "
                << GetLastError();
        }
    }

    for (int i = 0; i < n; i++) {
        calcThread[i] = CreateThread(
            // атрибуты безопасности по умолчанию
            NULL,
            // размер стека по умолчанию
            0,
            // имя функции				  
            find_sum,
            // указатель на параметры		  
            &(row_numbers[i]),
            // флаг создания = 0
            0,
            // адрес переменной для идентификатора			
            &(calcThreadID[i]));

        // Проверям - создан ли поток
        if (calcThread[i] == NULL)
        {
            std::cout << "Поток №s " << i
                << "не был создан\n"
                << "Ошибка "
                << GetLastError();
        }
    }

    WaitForMultipleObjects(n, fillThread, TRUE, INFINITE);
    WaitForMultipleObjects(n, calcThread, TRUE, INFINITE);

    cout << "\nСформированная матрица:" << endl;
    cout << fixed << setprecision(1);
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            cout << setw(6) << mtx[i][j] << " ";
        }
        cout << endl;
    }
    cout << endl;

    // Вывод результата
    cout << "Сумма: " << sum << '\n';

    // Закрытие потоков
    for (int i = 0; i < n; i++) {
        CloseHandle(fillThread[i]);
        CloseHandle(calcThread[i]);
    }

    DeleteCriticalSection(&cs);

    return 0;
}
