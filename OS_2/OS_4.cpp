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
    string value;
};

DWORD WINAPI push_deque(LPVOID param) {

    ThreadData* data = (ThreadData*)param;
    srand(time(NULL));

    if (rand() % 2 == 0) {
        EnterCriticalSection(&cs);
        deq.push_front(data->value);
        LeaveCriticalSection(&cs);
    }
    else {
        EnterCriticalSection(&cs);
        deq.push_back(data->value);
        LeaveCriticalSection(&cs);
    }

    return 0;
}

int main() {

    setlocale(LC_ALL, "rus");

    ifstream file("file.txt");
    string line;

    vector<string> values;
    string x;
    while (file >> x)
        values.push_back(x);
    file.close();

    int n = (int)values.size();
    HANDLE* hThread = new HANDLE[n];	 //массив потоков
    DWORD* dwThreadID = new DWORD[n]; //массив идентификаторов

    InitializeCriticalSection(&cs);

    for (int i = 0; i < n; i++) {
        ThreadData* data = new ThreadData{ values[i] };

        hThread[i] = CreateThread(NULL, 0, push_deque, data, 0, &dwThreadID[i]);

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