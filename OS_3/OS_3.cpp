#include <windows.h>
#include <iostream>
#include <fstream>
#include <bitset>
#include <string>
using namespace std;

int sizeBuffer;
char* buffer;
int blockSize = 0;
bool finished = false;
string key;
char* bytes = NULL;
int byteCount = 0;
HANDLE hMutex, mutexRead, mutexDecrypt, mutexWrite;

void BinaryStringToBytes(const string& binText, char*& bytes, int& n) {
    n = (binText.length() / 8); 
    bytes = new char[n];
    for (int i = 0; i < n; ++i) {
        string byteStr = binText.substr(i * 8, 8); //бинарное представление
        bytes[i] = (char)(stoi(byteStr, NULL, 2));
    }
}

DWORD WINAPI ReaderThread(LPVOID param) {
    int pos = 0;
    while (true) {
        WaitForSingleObject(mutexRead, INFINITE);
        WaitForSingleObject(hMutex, INFINITE);

        blockSize = min(sizeBuffer, byteCount - pos);
        if (blockSize == 0) {
            finished = true;
            ReleaseMutex(hMutex);
            ReleaseMutex(mutexDecrypt);
            break;
        }
        for (int i = 0; i < blockSize; ++i) {
            buffer[i] = bytes[pos + i];
            //cout << buffer[i];
        }
        pos += blockSize;

        ReleaseMutex(hMutex);
        ReleaseMutex(mutexDecrypt);
    }
    return 0;
}

DWORD WINAPI DecryptorThread(LPVOID param) {
    while (true) {
        WaitForSingleObject(mutexDecrypt, INFINITE);
        WaitForSingleObject(hMutex, INFINITE);

        if (blockSize > 0) {

            //применение ключа (исключающее ИЛИ - обратно)
            for (int i = 0; i < blockSize; ++i) {
                buffer[i] = buffer[i] ^ key[i % key.length()];
            }
            //перестановка символов в блоке
            for (int i = 0; i < blockSize / 2; ++i) {
                swap(buffer[i], buffer[blockSize - i - 1]);
            }
            ReleaseMutex(hMutex);
            ReleaseMutex(mutexWrite);
        }
        if (finished) {
            ReleaseMutex(hMutex);
            ReleaseMutex(mutexWrite);
            break;
        }
        ReleaseMutex(hMutex);
        ReleaseMutex(mutexWrite);
    }
    return 0;
}

DWORD WINAPI WriterThread(LPVOID param) {
    ofstream file("output.txt", ios::binary);
    while (true) {
        WaitForSingleObject(mutexWrite, INFINITE);
        WaitForSingleObject(hMutex, INFINITE);

        if (blockSize > 0) {
            file.write(buffer, blockSize);
            blockSize = 0;
            ReleaseMutex(hMutex);
            ReleaseMutex(mutexRead);
        }
        if (finished) {
            ReleaseMutex(hMutex);
            ReleaseMutex(mutexRead);
            break;
        }
        ReleaseMutex(hMutex);
        ReleaseMutex(mutexRead);
    }
    file.close();
    return 0;
}


int main() {
    setlocale(LC_ALL, "Russian");
    ifstream file("input.txt", ios::in);
    string binText((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
    BinaryStringToBytes(binText, bytes, byteCount);
    cout << "Введите ключ: ";
    getline(cin, key);
    int k = key.length();
    sizeBuffer = key.length();
    buffer = new char[sizeBuffer]();

    cout << "Зашифрованный текст: ";

    mutexRead = CreateMutex(NULL, false, NULL);
    mutexDecrypt = CreateMutex(NULL, false, NULL);
    mutexWrite = CreateMutex(NULL, false, NULL);
    hMutex = CreateMutex(NULL, false, NULL);

    HANDLE hRead = CreateThread(NULL, 0, ReaderThread, NULL, 0, NULL);
    HANDLE hDecrypt = CreateThread(NULL, 0, DecryptorThread, NULL, 0, NULL);
    HANDLE hWrite = CreateThread(NULL, 0, WriterThread, NULL, 0, NULL);

    HANDLE hThread[3];
    hThread[0] = hRead;
    hThread[1] = hDecrypt;
    hThread[2] = hWrite;

    // Дожидаемся завершения потоков
    WaitForMultipleObjects(3, hThread, true, INFINITE);

    CloseHandle(hRead);
    CloseHandle(hDecrypt);
    CloseHandle(hWrite);

    cout << "\nРезультат дешифрования в файле output.txt";
    delete[] buffer;
    delete[] bytes;
    return 0;
}