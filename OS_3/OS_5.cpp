/*
5. Модифицировать задачу 4 таким образом, чтобы количество строк в таблицы и основание хеш-функции определялось пользователем.
Доступ к строкам хеш-таблицы организовать на основании модели монитора:

class CMonitor
{
  int * occupated_rows;
  …
  public:
  void OcupateRow(int row_number);
  void FreeRow(int row_number);
  …
}

Опишите методы захвата строки OcupateRow и освобождения строки FreeRow. Используйте необходимое количество мьютексов.
*/

#include <windows.h>
#include <iostream>
#include <time.h>
#include <string>
using namespace std;

struct Node {
    int value;
    Node* next;
    Node(int v) : value(v), next(NULL) {}
};

int numbersCount;
int hashBase;
int tableSize;
HANDLE hmutex;
Node** hashTable;

class CMonitor {
    HANDLE* mutexes;
    int size;

public:
    CMonitor(int n) : size(n) {
        mutexes = new HANDLE[size];
        for (int i = 0; i < size; ++i) {
            mutexes[i] = CreateMutex(NULL, false, NULL);
        }
    }

    void OcupateRow(int rowNumber) {
        WaitForSingleObject(mutexes[rowNumber], INFINITE);
    }

    void FreeRow(int rowNumber) {
        ReleaseMutex(mutexes[rowNumber]);
    }

    void Close() {
        for (int i = 0; i < size; ++i) {
            CloseHandle(mutexes[i]);
        }
        delete[] mutexes;
    }
};

CMonitor* monitor;

int Hash(int x) {
    return x % hashBase;
}

DWORD WINAPI InsertHashTable(LPVOID lpParam) {
    for (int i = 0; i < numbersCount; ++i) {
        srand(time(NULL) + GetCurrentThreadId() + i);
        int num = (rand() % 100);
        int numLine = Hash(num);

        (*monitor).OcupateRow(numLine);

        Node* n = new Node(num);
        (*n).next = hashTable[numLine];
        hashTable[numLine] = n;

        WaitForSingleObject(hmutex, INFINITE);
        cout << "Поток " << GetCurrentThreadId() << " вставил число " << num << " в строку " << numLine << endl;
        ReleaseMutex(hmutex);
        (*monitor).FreeRow(numLine);
    }
    return 0;
}

int main() {
    setlocale(LC_ALL, "Russian");
    int n;
    cout << "Введите количество потоков: ";
    cin >> n;
    cout << "Введите количество чисел, которое должен сгенерировать поток: ";
    cin >> numbersCount;

    cout << "Введите основание хэш-функции: ";
    cin >> hashBase;

    cout << "Введите количество строк таблицы: ";
    cin >> tableSize;

    hashTable = new Node * [tableSize];
    for (int i = 0; i < tableSize; ++i) {
        hashTable[i] = NULL;
    }
    monitor = new CMonitor(tableSize);

    hmutex = CreateMutex(NULL, false, NULL);
    HANDLE* threads = new HANDLE[n];

    for (int i = 0; i < n; i++) {
        threads[i] = CreateThread(NULL, 0, InsertHashTable, NULL, 0, NULL);
    }
    WaitForMultipleObjects(n, threads, true, INFINITE);

    cout << "Хеш-таблица:" << endl;
    for (int i = 0; i < tableSize; ++i) {
        cout << i << ": ";
        for (Node* curr = hashTable[i]; curr; curr = (*curr).next) {
            cout << (*curr).value << " ";
        }
        cout << endl;
    }

    for (int i = 0; i < n; ++i) {
        CloseHandle(threads[i]);
    }
    CloseHandle(hmutex);

    return 0;
}