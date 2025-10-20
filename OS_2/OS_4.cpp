#include <windows.h>
#include <iostream>
#include <time.h>
#include <locale>
#include <deque>
#include <fstream>
#include <string>
#include <vector>
using namespace std;

CRITICAL_SECTION cs;
deque<string> deq;

struct ThreadData {
    string* elements;
    int count;
};

DWORD WINAPI push_deque(LPVOID param) {

    ThreadData* data = (ThreadData*)param;
    srand(time(NULL));

    for (int i = 0; i < data->count; i++) {
        if (rand() % 2 == 0) {
            EnterCriticalSection(&cs);
            deq.push_front(data->elements[i]);
            LeaveCriticalSection(&cs);
        }
        else {
            EnterCriticalSection(&cs);
            deq.push_back(data->elements[i]);
            LeaveCriticalSection(&cs);
        }
    }

    return 0;
}

int main() {

    setlocale(LC_ALL, "rus");

    const int n = 5;
    HANDLE hThread[n];	 //массив потоков
    DWORD dwThreadID[n]; //массив идентификаторов

    ifstream file("file.txt");
    string line;

    // Считаем количество элементов в файле
    int total_elements = 0;
    while (getline(file, line)) {
        if (!line.empty()) total_elements++;
    }
    file.close();

    // Читаем элементы в массив
    string* elements = new string[total_elements];
    file.open("file.txt");
    int index = 0;
    while (getline(file, line)) {
        if (!line.empty()) elements[index++] = line;
    }
    file.close();

    // Распределяем элементы между n потоками
    ThreadData* threadData = new ThreadData[n];
    int* elements_per_thread = new int[n](); // Инициализируем нулями

    // Считаем сколько элементов будет у каждого потока
    for (int i = 0; i < total_elements; i++) {
        elements_per_thread[i % n]++;
    }

    // Выделяем память для элементов каждого потока
    for (int i = 0; i < n; i++) {
        threadData[i].elements = new string[elements_per_thread[i]];
        threadData[i].count = elements_per_thread[i];
    }

    // Заполняем массивы элементов для потоков
    int* current_index = new int[n]();
    for (int i = 0; i < total_elements; i++) {
        int thread_idx = i % n;
        threadData[thread_idx].elements[current_index[thread_idx]++] = elements[i];
    }

    InitializeCriticalSection(&cs);

    for (int i = 0; i < n; i++) {

        hThread[i] = CreateThread(NULL, 0, push_deque, &threadData[i], 0, &dwThreadID[i]);

        if (hThread[i] == NULL) {
            cout << "Ошибка создания потока " << GetLastError() << '\n';
        }
    }

    // Ожидание завершения потоков
    WaitForMultipleObjects(n, hThread, TRUE, INFINITE);

    cout << "Итоговый дек: ";
    for (string el : deq) {
        cout << el << " ";
    }

    // Закрытие потоков
    for (int i = 0; i < n; i++) {
        CloseHandle(hThread[i]);
    }

    // Удаление критической секции
    DeleteCriticalSection(&cs);

    return 0;
}
