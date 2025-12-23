#include <iostream>
#include <windows.h>
#include <fstream>
#include <string>
#include <cmath>
#include <cstdlib>
#include <ctime>
using namespace std;

// 1. Умножение больших чисел
void MultiplyBigNumbers() {
    double num1, num2;
    cout << "Введите первое число: ";
    cin >> num1;
    cout << "Введите второе число: ";
    cin >> num2;
    cout << "Результат: " << num1 << " * " << num2 << " = " << num1 * num2 << endl;
}

// 2. Удаление дубликатов из массива
void RemoveDuplicates() {
    string filename;
    cout << "Введите имя файла: ";
    cin >> filename;

    ifstream file(filename);
    if (!file) {
        cout << "Ошибка открытия файла!" << endl;
        return;
    }

    // Читаем все числа в массив
    const int MAX_SIZE = 1000;
    double numbers[MAX_SIZE];
    int count = 0;

    while (file >> numbers[count] && count < MAX_SIZE) {
        count++;
    }
    file.close();

    if (count == 0) {
        cout << "Файл пуст!" << endl;
        return;
    }

    // Ищем уникальные числа
    double unique[MAX_SIZE];
    int unique_count = 0;

    for (int i = 0; i < count; i++) {
        bool found = false;
        for (int j = 0; j < unique_count; j++) {
            if (abs(numbers[i] - unique[j]) < 0.000001) {
                found = true;
                break;
            }
        }
        if (!found) {
            unique[unique_count] = numbers[i];
            unique_count++;
        }
    }

    // Сохраняем результат
    ofstream out("output.txt");
    cout << "Уникальные числа: ";
    for (int i = 0; i < unique_count; i++) {
        cout << unique[i] << " ";
        out << unique[i] << " ";
    }
    cout << "\nСохранено в output.txt" << endl;
    out.close();
}

// 3. Самое частое слово в тексте
void MostFrequentWord() {
    string filename;
    cout << "Введите имя файла: ";
    cin >> filename;

    ifstream file(filename);
    if (!file) {
        cout << "Ошибка открытия файла!" << endl;
        return;
    }

    // Простой подсчет слов
    string word;
    string words[1000];
    int counts[1000] = { 0 };
    int word_count = 0;

    while (file >> word) {
        // Очищаем слово от знаков препинания
        string clean_word;
        for (char c : word) {
            if (isalpha(c)) {
                clean_word += tolower(c);
            }
        }

        if (clean_word.empty()) continue;

        // Ищем слово в массиве
        int index = -1;
        for (int i = 0; i < word_count; i++) {
            if (words[i] == clean_word) {
                index = i;
                break;
            }
        }

        if (index == -1) {
            // Новое слово
            words[word_count] = clean_word;
            counts[word_count] = 1;
            word_count++;
        }
        else {
            // Уже есть
            counts[index]++;
        }
    }
    file.close();

    if (word_count == 0) {
        cout << "Нет слов в файле!" << endl;
        return;
    }

    // Находим самое частое слово
    int max_count = 0;
    string most_frequent;

    for (int i = 0; i < word_count; i++) {
        if (counts[i] > max_count) {
            max_count = counts[i];
            most_frequent = words[i];
        }
    }

    cout << "Самое частое слово: '" << most_frequent << "'" << endl;
    cout << "Встречается " << max_count << " раз" << endl;
}

// 4. Матрица
void MatrixMinMax() {
    int n;
    cout << "Введите размер матрицы: ";
    cin >> n;

    if (n <= 0) {
        cout << "Неверный размер!" << endl;
        return;
    }

    // Создаем матрицу
    int** matrix = new int* [n];
    for (int i = 0; i < n; i++) {
        matrix[i] = new int[n];
    }

    // Заполняем случайными числами
    srand(time(0));
    cout << "\nМатрица " << n << "x" << n << ":\n";
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            matrix[i][j] = rand() % 100;
            cout << matrix[i][j] << "\t";
        }
        cout << endl;
    }

    // Минимальный ниже главной диагонали
    int min_below = 1000;
    bool found_min = false;
    for (int i = 1; i < n; i++) {
        for (int j = 0; j < i; j++) {
            if (matrix[i][j] < min_below) {
                min_below = matrix[i][j];
                found_min = true;
            }
        }
    }

    // Максимальный выше побочной диагонали
    int max_above = -1;
    bool found_max = false;
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            if (matrix[i][j] > max_above) {
                max_above = matrix[i][j];
                found_max = true;
            }
        }
    }

    cout << "\nРезультаты:\n";
    if (found_min) {
        cout << "Мин ниже главной диагонали: " << min_below << endl;
    }
    else {
        cout << "Нет элементов ниже главной диагонали" << endl;
    }

    if (found_max) {
        cout << "Макс выше побочной диагонали: " << max_above << endl;
    }
    else {
        cout << "Нет элементов выше побочной диагонали" << endl;
    }

    // Очищаем память
    for (int i = 0; i < n; i++) {
        delete[] matrix[i];
    }
    delete[] matrix;
}

// 5. Решето Эратосфена
void SieveOfEratosthenes() {
    int n;
    cout << "Введите n: ";
    cin >> n;

    if (n < 2) {
        cout << "Нет простых чисел до " << n << endl;
        return;
    }

    // Создаем массив
    bool* is_prime = new bool[n + 1];
    for (int i = 0; i <= n; i++) {
        is_prime[i] = true;
    }
    is_prime[0] = is_prime[1] = false;

    // Решето
    for (int i = 2; i * i <= n; i++) {
        if (is_prime[i]) {
            for (int j = i * i; j <= n; j += i) {
                is_prime[j] = false;
            }
        }
    }

    // Вывод
    cout << "Простые числа: ";
    int count = 0;
    for (int i = 2; i <= n; i++) {
        if (is_prime[i]) {
            cout << i << " ";
            count++;
        }
    }
    cout << "\nВсего: " << count << " чисел" << endl;

    delete[] is_prime;
}

int main(int argc, char* argv[]) {
    setlocale(LC_ALL, "Russian");
    HANDLE hEvent = OpenEvent(EVENT_MODIFY_STATE, false, L"Event");
    if (argc != 2) {
        cout << "Использование: Work.exe <номер_задачи>" << endl;
        return 1;
    }

    int task = atoi(argv[1]);

    switch (task) {
    case 1: MultiplyBigNumbers(); break;
    case 2: RemoveDuplicates(); break;
    case 3: MostFrequentWord(); break;
    case 4: MatrixMinMax(); break;
    case 5: SieveOfEratosthenes(); break;
    default: cout << "Неверный номер!" << endl;
    }

    SetEvent(hEvent);
    CloseHandle(hEvent);

    return 0;
}