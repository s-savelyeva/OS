#include <windows.h>
#include <iostream>
#include <cmath>
#include <time.h>
#include <locale>

CRITICAL_SECTION cs;
double total_area = 0.0;

double f(double x) {
    // Функция y = x^3 + 1
    return x * x * x + 1;
}

struct ThreadParams {
    double a;      // левая граница отрезка
    double b;      // правая граница отрезка
    int m;         // количество разбиений для метода прямоугольников
};

DWORD WINAPI calculate_area(LPVOID param) {
    ThreadParams* params = (ThreadParams*)param;
    double local_sum = 0.0;

    // Шаг для метода прямоугольников
    double dx = (params->b - params->a) / params->m;

    // Вычисление площади методом прямоугольников (средних)
    for (int i = 0; i < params->m; i++) {
        double x = params->a + (i + 0.5) * dx;  // метод среднего прямоугольника
        EnterCriticalSection(&cs);
        total_area += f(x) * dx;
        LeaveCriticalSection(&cs);

    }

    return 0;
}

int main() {

    setlocale(LC_ALL, "rus");
    // Параметры задачи
    const double a = 0.0;      // левая граница отрезка
    const double b = 2.0;      // правая граница отрезка
    const int n = 10;          // количество элементов разбиения
    const int m = 1000;        // количество разбиений для метода прямоугольников

    HANDLE hThread[n];
    DWORD dwThreadID[n];
    ThreadParams params[n];

    // Инициализация критической секции
    InitializeCriticalSection(&cs);
    total_area = 0.0;

    // Вычисление шага для разбиения на n элементов
    double segment_width = (b - a) / n;

    // Запуск потоков
    for (int i = 0; i < n; i++) {
        // Установка параметров для каждого потока
        params[i].a = a + i * segment_width;
        params[i].b = a + (i + 1) * segment_width;
        params[i].m = m;

        hThread[i] = CreateThread(NULL, 0, calculate_area, &params[i], 0, &dwThreadID[i]);

        if (hThread[i] == NULL) {
            std::cout << "Ошибка создания потока " << GetLastError() << '\n';
            return 1;
        }
    }

    // Ожидание завершения потоков
    WaitForMultipleObjects(n, hThread, TRUE, INFINITE);

    // Вывод результатов
    std::cout << "Приближенная площадь криволинейной трапеции = " << total_area << '\n';

    // Закрытие потоков
    for (int i = 0; i < n; i++) {
        CloseHandle(hThread[i]);
    }

    // Удаление критической секции
    DeleteCriticalSection(&cs);

    return 0;
}
