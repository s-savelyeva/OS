/*
2. Смоделировать механизм рандеву для решения задачи нахождения значения функции ex по ее разложению в ряд Маклорена. 
В качестве вызывающей задачи взять функцию нахождения суммы элементов ряда, а в качестве обслуживающих задач – функцию нахождения факториала 
и функцию нахождения степени аргумента.
*/

#include <windows.h>
#include <iostream>
#include <cmath>
#include <iomanip>
#include <locale.h>

using namespace std;

// Глобальные переменные для синхронизации
HANDLE hMutex;           // Мьютекс для защиты общих данных
HANDLE hEventFactorial;  // Событие - факториал готов
HANDLE hEventPower;      // Событие - степень готова
HANDLE hEventSumReady;   // Событие - запрос на новый член ряда

// Общие данные для рандеву
double x = 0.0;          // Аргумент функции
int n = 0;               // Номер текущего члена ряда
double power_result = 0.0;   // Результат вычисления степени
long long factorial_result = 0; // Результат вычисления факториала
double term_result = 0.0;    // Результат вычисления члена ряда
double total_sum = 0.0;      // Итоговая сумма ряда
bool calculation_complete = false; // Флаг завершения расчета

// Функция вычисления степени x^n
DWORD WINAPI PowerThread(LPVOID param) {
    while (!calculation_complete) {
        // Ждем запроса на вычисление степени
        WaitForSingleObject(hEventSumReady, INFINITE);

        if (calculation_complete) break;

        // Вычисляем степень x^n
        WaitForSingleObject(hMutex, INFINITE);
        power_result = pow(x, n);
        ReleaseMutex(hMutex);

        // Сигнализируем, что степень готова
        SetEvent(hEventPower);
    }
    return 0;
}

// Функция вычисления факториала n!
DWORD WINAPI FactorialThread(LPVOID param) {
    while (!calculation_complete) {
        // Ждем запроса на вычисление факториала
        WaitForSingleObject(hEventSumReady, INFINITE);

        if (calculation_complete) break;

        // Вычисляем факториал n!
        WaitForSingleObject(hMutex, INFINITE);
        factorial_result = 1;
        for (int i = 1; i <= n; i++) {
            factorial_result *= i;
        }
        ReleaseMutex(hMutex);

        // Сигнализируем, что факториал готов
        SetEvent(hEventFactorial);
    }
    return 0;
}

// Основная функция - вычисление суммы ряда
DWORD WINAPI SumThread(LPVOID param) {
    int max_terms = *(int*)param;

    cout << "\nВычисление e^" << x << " с использованием " << max_terms << " членов ряда:\n";

    for (n = 0; n < max_terms; n++) {
        
        // Запрашиваем вычисление нового члена ряда
        ResetEvent(hEventPower);
        ResetEvent(hEventFactorial);
        SetEvent(hEventSumReady);

        // Ждем, пока член ряда будет готов
        WaitForSingleObject(hEventPower, INFINITE);
        WaitForSingleObject(hEventFactorial, INFINITE);

        // Добавляем член к сумме
        WaitForSingleObject(hMutex, INFINITE);
        term_result = power_result / factorial_result;
        total_sum += term_result;
        // Выводим информацию о текущем члене
        cout << "n = " << n << ": " << x << "^" << n << "/" << n << "! = "
            << fixed << setprecision(10) << term_result
            << ", текущая сумма: " << total_sum << endl;
        ReleaseMutex(hMutex);

    }

    // Сигнализируем о завершении расчета
    calculation_complete = true;
    SetEvent(hEventSumReady); // Разбудить потоки для выхода

    return 0;
}

int main() {
    setlocale(LC_ALL, "Rus");

    // Ввод данных
    cout << "Вычисление e^x по разложению в ряд Маклорена\n";
    cout << "Ряд Маклорена: e^x = 1 + x + x^2/2! + x^3/3! + ...\n\n";

    cout << "Введите значение x: ";
    cin >> x;

    int max_terms;
    cout << "Введите количество членов ряда для вычисления: ";
    cin >> max_terms;

    // Создание объектов синхронизации
    hMutex = CreateMutex(NULL, FALSE, NULL);
    hEventFactorial = CreateEvent(NULL, TRUE, FALSE, NULL);
    hEventPower = CreateEvent(NULL, TRUE, FALSE, NULL);
    hEventSumReady = CreateEvent(NULL, TRUE, FALSE, NULL);

    // Создание потоков
    HANDLE hThreads[3];

    hThreads[0] = CreateThread(NULL, 0, PowerThread, NULL, 0, NULL);
    hThreads[1] = CreateThread(NULL, 0, FactorialThread, NULL, 0, NULL);
    hThreads[2] = CreateThread(NULL, 0, SumThread, &max_terms, 0, NULL);

    // Ожидание завершения всех потоков
    WaitForMultipleObjects(3, hThreads, TRUE, INFINITE);

    // Вывод результатов
    cout << "Итоговый результат:\n";
    cout << "Сумма ряда (" << max_terms << " членов): " << fixed << setprecision(15) << total_sum << endl;
    cout << "Значение exp(" << x << ") из библиотеки math.h: " << exp(x) << endl;

    // Закрытие дескрипторов
    for (int i = 0; i < 3; i++) {
        CloseHandle(hThreads[i]);
    }

    CloseHandle(hMutex);
    CloseHandle(hEventFactorial);
    CloseHandle(hEventPower);
    CloseHandle(hEventSumReady);

    return 0;
}