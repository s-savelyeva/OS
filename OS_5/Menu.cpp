#include <iostream>
#include <windows.h>
using namespace std;

int main() {
    setlocale(LC_ALL, "Russian");

    HANDLE hEvent = CreateEvent(NULL, false, false, L"Event");

    while (true) {
        cout << "\nМеню:\n";
        cout << "1: Перемножить 2 больших числа\n";
        cout << "2: Удалить дубликаты из массива\n";
        cout << "3: Самое частое слово в тексте\n";
        cout << "4: Мин/макс в квадратной матрице\n";
        cout << "5: Простые числа (Решето Эратосфена)\n";
        cout << "0: Выход\n";
        cout << "\nВыберите действие: ";
        int choice;
        cin >> choice;
        if (choice == 0) {
            break;
        }

        char cmdLine[256];
        sprintf_s(cmdLine, sizeof(cmdLine), "Work.exe %d", choice); //строка команды

        STARTUPINFOA si = { sizeof(si) };
        PROCESS_INFORMATION pi;  //дескриптор процесса, глав потока, идентификаторы

        BOOL ok = CreateProcessA(NULL, cmdLine, NULL, NULL, false, 0, NULL, NULL, &si, &pi);

        if (!ok) {
            cout << "Ошибка запуска второй программы\n";
            continue;
        }

        DWORD result = WaitForSingleObject(hEvent, INFINITE);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
    CloseHandle(hEvent);
    return 0;
}