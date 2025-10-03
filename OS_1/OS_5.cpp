#include <windows.h>
#include <iostream>
#include <locale.h>
#include <string>
#include<cstring>
using namespace std;

struct ThreadParams {
    string key;
    char* text;
    int k;
};

// Функция для обращения строки (перестановка символов)
string reverseString(string s, int k) {
    string str = s.substr(0, k);
    reverse(str.begin(), str.end());
    return str;
}

// Функция для основного тела нити
DWORD WINAPI EncryptBlock(LPVOID param) {
    ThreadParams* params = (ThreadParams*) param;
    char* text = params->text;
    string key = params->key;
    int k = params->k;

    // Выполняем перестановку символов в блоке
    string reversedBlock = reverseString(text, k);

    // Применяем XOR с ключом
    for (int i = 0; i < k; i++) {
        (params->text)[i] = reversedBlock[i] ^ key[i];
    }

    return 0;
}

// Функция для основного тела нити
DWORD WINAPI DecryptBlock(LPVOID param) {
    ThreadParams* params = (ThreadParams*)param;
    string key = params->key;
    int k = params->k;

    // Применяем XOR с ключом
    for (int i = 0; i < k; i++) {
        (params->text)[i] = (params->text)[i] ^ key[i];
    }

    for (int i = 0; i < k/2; i++) {
        swap(params->text[i], params->text[k - i - 1]);
    }

    return 0;
}

// Главная функция для запуска процесса шифрования
string ParallelEncrypt(string text, string key, int blocks, int k, int n) {
    HANDLE* hThread = new HANDLE[blocks];
    DWORD* dwThreadID = new DWORD[blocks];
    ThreadParams* params = new ThreadParams[blocks];

    // Цикл распределения заданий между нитями
    for (int i = 0; i < blocks; i++) {

        int block_size;

        if ((i == blocks - 1) && (n % k != 0)) {
            block_size = n % k;
        }
        else block_size = k;

        params[i].key = key;
        params[i].text = &text[i*k];
        params[i].k = block_size;

        hThread[i] = CreateThread(
            // атрибуты безопасности по умолчанию
            NULL,
            // размер стека по умолчанию
            0,
            // имя функции
            EncryptBlock,
            // указатель на параметры
            &(params[i]),
            // флаг создания = 0
            0,
            // адрес переменной для идентификатора
            &(dwThreadID[i])
        );

        if (hThread[i] == NULL) {
            cout << "Поток № " << i
                << "не был создан\n"
                << "Ошибка "
                << GetLastError();
        }
    }

    // Ожидание завершения всех нитей
    WaitForMultipleObjects(blocks, hThread, true, INFINITE);

    for (int i = 0; i < blocks; i++) {
        CloseHandle(hThread[i]);
    }

    return text;
}

// Главная функция для запуска процесса шифрования
string ParallelDecrypt(string text, string key, int blocks, int k, int n) {
    HANDLE* hThread = new HANDLE[blocks];
    DWORD* dwThreadID = new DWORD[blocks];
    ThreadParams* params = new ThreadParams[blocks];

    // Цикл распределения заданий между нитями
    for (int i = 0; i < blocks; i++) {

        int block_size;

        if ((i == blocks - 1) && (n % k != 0)) {
            block_size = n % k;
        }
        else block_size = k;

        params[i].key = key;
        params[i].text = &text[i * k];
        params[i].k = block_size;

        hThread[i] = CreateThread(
            // атрибуты безопасности по умолчанию
            NULL,
            // размер стека по умолчанию
            0,
            // имя функции
            DecryptBlock,
            // указатель на параметры
            &(params[i]),
            // флаг создания = 0
            0,
            // адрес переменной для идентификатора
            &(dwThreadID[i])
        );

        if (hThread[i] == NULL) {
            cout << "Поток № " << i
                << "не был создан\n"
                << "Ошибка "
                << GetLastError();
        }
    }

    // Ожидание завершения всех нитей
    WaitForMultipleObjects(blocks, hThread, true, INFINITE);

    for (int i = 0; i < blocks; i++) {
        CloseHandle(hThread[i]);
    }

    return text;
}

int main() {
    setlocale(LC_ALL, "Russian");
    string text;
    string key;

    cout << "Введите текст: ";
    cin >> text;
    cout << "Введите ключ: ";
    cin >> key;
    int k = key.length();
    int n = text.length();

    const int blocks = (n + k - 1) / k;

    string encryptedText = ParallelEncrypt(text, key, blocks, k, n);

    cout << "Исходный текст: " << text << endl;
    cout << "Зашифрованный текст: ";
    for (char ch : encryptedText) {
        cout << (int)ch;
    }
    cout << endl;

    string decryptedText = ParallelDecrypt(encryptedText, key, blocks, k, n);
    cout << "Расшифрованный текст: " << decryptedText << endl;

    return 0;
}
