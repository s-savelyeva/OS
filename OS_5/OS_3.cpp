#include <windows.h>
#include <iostream>
#include <fstream>
#include <locale.h>
#include <string>

using namespace std;

// Глобальные переменные для синхронизации
HANDLE hMutex;                      // Мьютекс для защиты общих данных
HANDLE hEventDataReady[3];          // События - данные от потоков чтения готовы
HANDLE hEventDataProcessed[3];      // События - данные обработаны, можно читать следующее
HANDLE hEventMergeComplete;         // Событие - слияние завершено

// Общие данные для потоков чтения
ifstream files[3];                  // Входные файлы
ofstream output_file;               // Выходной файл
int current_values[3];              // Текущие значения из файлов
bool has_more_data[3];              // Флаги наличия данных
bool merge_complete = false;

// Функция потока чтения данных из файла
DWORD WINAPI FileReaderThread(LPVOID param) {
    int thread_id = *(int*)param;
    delete (int*)param;

    // Читаем первое значение из файла
    WaitForSingleObject(hMutex, INFINITE);
    if (files[thread_id] >> current_values[thread_id]) {
        has_more_data[thread_id] = true;
    }
    else {
        has_more_data[thread_id] = false;
    }
    ReleaseMutex(hMutex);

    // Сигнализируем, что первое значение готово
    SetEvent(hEventDataReady[thread_id]);

    while (true) {
        // Ждем, пока обработчик обработает предыдущее значение
        WaitForSingleObject(hEventDataProcessed[thread_id], INFINITE);

        // Проверяем, не завершена ли работа
        WaitForSingleObject(hMutex, INFINITE);
        if (merge_complete) {
            ReleaseMutex(hMutex);
            break;
        }
        ReleaseMutex(hMutex);

        // Сбрасываем событие обработки
        ResetEvent(hEventDataProcessed[thread_id]);

        // Читаем следующее значение
        WaitForSingleObject(hMutex, INFINITE);
        int value;
        if (files[thread_id] >> value) {
            current_values[thread_id] = value;
            has_more_data[thread_id] = true;
        }
        else {
            has_more_data[thread_id] = false;
        }
        ReleaseMutex(hMutex);

        // Сигнализируем, что данные готовы
        SetEvent(hEventDataReady[thread_id]);
    }

    return 0;
}

// Функция потока формирования результирующего файла
DWORD WINAPI MergeThread(LPVOID param) {

    ofstream output_file("output.txt");

    // Ждем, пока все потоки чтения прочитают первые значения
    WaitForMultipleObjects(3, hEventDataReady, TRUE, INFINITE);

    while (true) {
        WaitForSingleObject(hMutex, INFINITE);

        // Если все файлы закончились, завершаем работу
        if (!has_more_data[0] && !has_more_data[1] && !has_more_data[2]) {
            merge_complete = true;
            ReleaseMutex(hMutex);
            break;
        }

        // Находим минимальное значение из доступных
        int min_value = 0;
        int selected_thread = -1;

        // Ищем первое доступное значение
        for (int i = 0; i < 3; i++) {
            if (has_more_data[i]) {
                min_value = current_values[i];
                selected_thread = i;
                break;
            }
        }

        // Если не нашли доступных значений
        if (selected_thread == -1) {
            merge_complete = true;
            ReleaseMutex(hMutex);
            break;
        }

        // Сравниваем с другими доступными значениями
        for (int i = 0; i < 3; i++) {
            if (has_more_data[i] && current_values[i] < min_value) {
                min_value = current_values[i];
                selected_thread = i;
            }
        }

        // Записываем минимальное значение в выходной файл
        output_file << min_value << " ";

        // Помечаем это значение как использованное
        has_more_data[selected_thread] = false;

        ReleaseMutex(hMutex);

        // Сигнализируем соответствующему потоку, что нужно прочитать следующее значение
        ResetEvent(hEventDataReady[selected_thread]);  // Сбрасываем готовность
        SetEvent(hEventDataProcessed[selected_thread]); // Разрешаем чтение следующего

        // Ждем, пока поток прочитает следующее значение
        WaitForSingleObject(hEventDataReady[selected_thread], INFINITE);
    }

    // Завершаем работу потоков чтения
    for (int i = 0; i < 3; i++) {
        SetEvent(hEventDataProcessed[i]);
    }

    SetEvent(hEventMergeComplete);

    return 0;
}

int main() {
    setlocale(LC_ALL, "Rus");

    // Инициализация объектов синхронизации
    hMutex = CreateMutex(NULL, FALSE, NULL);
    hEventMergeComplete = CreateEvent(NULL, TRUE, FALSE, NULL);

    // Инициализация событий и флагов
    for (int i = 0; i < 3; i++) {
        hEventDataReady[i] = CreateEvent(NULL, TRUE, FALSE, NULL);
        hEventDataProcessed[i] = CreateEvent(NULL, TRUE, FALSE, NULL);
        has_more_data[i] = false;
    }

    // Открываем входные файлы простым способом
    files[0].open("input1.txt");
    files[1].open("input2.txt");
    files[2].open("input3.txt");

    // Создаем потоки
    HANDLE hThreads[4];

    // Создаем потоки чтения
    for (int i = 0; i < 3; i++) {
        int* thread_id = new int(i);
        hThreads[i] = CreateThread(NULL, 0, FileReaderThread, thread_id, 0, NULL);
    }

    // Создаем поток слияния
    hThreads[3] = CreateThread(NULL, 0, MergeThread, NULL, 0, NULL);

    // Ждем завершения слияния
    WaitForSingleObject(hEventMergeComplete, INFINITE);

    // Ждем завершения всех потоков
    WaitForMultipleObjects(4, hThreads, TRUE, INFINITE);

    // Закрываем файлы
    for (int i = 0; i < 3; i++) {
        files[i].close();
    }

    cout << "Слияние завершено!\n";

    // Читаем и выводим результат
    ifstream result_file("output.txt");
    cout << "Результирующий файл (output.txt): ";
    int value;

    while (result_file >> value) {
        cout << value << " ";
    }
    cout << endl;

    // Очистка ресурсов
    for (int i = 0; i < 3; i++) {
        CloseHandle(hEventDataReady[i]);
        CloseHandle(hEventDataProcessed[i]);
    }

    CloseHandle(hEventMergeComplete);
    CloseHandle(hMutex);

    for (int i = 0; i < 4; i++) {
        CloseHandle(hThreads[i]);
    }

    return 0;
}