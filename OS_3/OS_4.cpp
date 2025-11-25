/*
4. Имеется n потоков, генерирующих случайные целые числа. 
Занести эти числа в хеш-таблицу, представляющую собой массив из 10 указателей на списки целых чисел. 
Для определения местоположения числа x в хеш-таблице использовать следующую хеш-функцию:

int hash(int x)
{
  return x % 10;
}

Если поток собирается записать число x в k-ю строку таблицы, то он должен проверить, не пишет ли другой поток свое число в эту же строку.
*/

#include <windows.h>
#include <iostream>
#include <ctime>
#include <string>
using namespace std;

struct Node {
    int value;
    Node* next;
    Node(int v) : value(v), next(NULL) {}
};

const int hashSize = 10;     
int numbersCount;
HANDLE mutexes[hashSize];    
HANDLE hmutex;
Node* hashTable[hashSize];

int Hash(int x) {
    return x % 10;
}

DWORD WINAPI InsertHashTable(LPVOID lpParam) {
    for (int i = 0; i < numbersCount; ++i) {
        srand(time(NULL) + GetCurrentThreadId() + i);
        int num = (rand() % 100);
        int numLine = Hash(num);

        WaitForSingleObject(mutexes[numLine], INFINITE);

        Node* n = new Node(num);
        (*n).next = hashTable[numLine];
        hashTable[numLine] = n;

        WaitForSingleObject(hmutex, INFINITE);
        cout << "Поток " << GetCurrentThreadId() << " вставил число " << num << " в строку " << numLine << endl;
        ReleaseMutex(hmutex);
        ReleaseMutex(mutexes[numLine]);
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

    for (int i = 0; i < hashSize; ++i) {
        hashTable[i] = NULL;
        mutexes[i] = CreateMutex(NULL, false, NULL); //Массив мьютексов
    }
    hmutex = CreateMutex(NULL, false, NULL);
    HANDLE* threads = new HANDLE[n];

    for (int i = 0; i < n; i++) {
        threads[i] = CreateThread(NULL, 0, InsertHashTable, NULL, 0, NULL);
    }
    WaitForMultipleObjects(n, threads, true, INFINITE);

    cout << "Хеш-таблица:" << endl;
    for (int i = 0; i < hashSize; ++i) {
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