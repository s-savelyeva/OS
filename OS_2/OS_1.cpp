#include <windows.h>
#include <iostream>
#include <string>
#include <locale>
using namespace std;

CRITICAL_SECTION cs;
int sum = 0;

struct ThreadParams {
    string text; // указатель на текст
    int thread_id;           // идентификатор потока
    int total_threads;       // общее количество потоков
};

// функция потока
DWORD WINAPI func(LPVOID param)
{
    ThreadParams* params = (ThreadParams*)param;
    string text = params->text;
    int thread_id = params->thread_id;
    int k = params->total_threads;

    int local_sum = 0;

    // Обрабатываем символы с номерами i + k*s
    for (int s = 0; thread_id + k * s < text.size(); s++)
    {
        int char_index = thread_id + k * s;
        EnterCriticalSection(&cs);
        sum += (int)text[char_index];
        LeaveCriticalSection(&cs);
    }

    return 0;
}

int main()
{
    setlocale(LC_ALL, "Rus");

    string text;
    cout << "Введите текст: ";
    getline(std::cin, text);
    int n = text.length(); // длина текста

    cout << "Коды символов текста: ";
    for (int i = 0; i < text.length(); i++) {
        cout << (int)text[i] << " ";
    }
    cout << endl;

    int k;
    cout << "Введите количество потоков (k < " << n << "): ";
    cin >> k;

    HANDLE* hThread = new HANDLE[k];      // массив потоков
    DWORD* dwThreadID = new DWORD[k];     // массив идентификаторов
    ThreadParams* params = new ThreadParams[k]; // массив параметров

    // Инициализация критической секции
    InitializeCriticalSection(&cs);
    sum = 0;

    // Запуск потоков
    for (int i = 0; i < k; i++)
    {
        params[i].text = text;
        params[i].thread_id = i;
        params[i].total_threads = k;

        hThread[i] = CreateThread(NULL, 0, func, &params[i], 0, &dwThreadID[i]);

        if (hThread[i] == NULL)
        {
            std::cout << "Ошибка создания потока " << GetLastError() << '\n';
        }
    }

    // Ожидание завершения потоков
    WaitForMultipleObjects(k, hThread, TRUE, INFINITE);

    // Вывод результатов
    std::cout << "Контрольная сумма текста = " << sum%256 << '\n';

    // Закрытие потоков и освобождение памяти
    for (int i = 0; i < k; i++)
        CloseHandle(hThread[i]);

    delete[] hThread;
    delete[] dwThreadID;
    delete[] params;
    DeleteCriticalSection(&cs);

    return 0;
}
