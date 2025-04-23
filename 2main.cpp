#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <mutex>
#include <iomanip>
#include <atomic>
#include <windows.h>
#include <random>

using namespace std;
using namespace chrono;

mutex cout_mutex;
const int progress_bar_length = 50;
const int refresh_rate_ms = 100; // Частота обновления (в миллисекундах)

struct ThreadData {
    atomic<int> progress{ 0 };
    atomic<bool> finished{ false };
    milliseconds duration{ 0 };
    thread::id id;
    int task_length; // Добавим длительность задачи для каждого потока
};

void calculate_task(int thread_num, vector<ThreadData>& threads_data) {
    auto start_time = high_resolution_clock::now();
    threads_data[thread_num].id = this_thread::get_id();

    // Генератор случайных чисел для задержки
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> delay_dist(10, 100); // Случайная задержка между шагами

    for (int i = 0; i <= progress_bar_length; ++i) {
        threads_data[thread_num].progress = i;
        this_thread::sleep_for(milliseconds(delay_dist(gen)));
    }

    auto end_time = high_resolution_clock::now();
    threads_data[thread_num].duration = duration_cast<milliseconds>(end_time - start_time);
    threads_data[thread_num].finished = true;
}

void display_progress(const vector<ThreadData>& threads_data) {
    // Получаем handle консоли для более плавного обновления
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hConsole, &cursorInfo);
    cursorInfo.bVisible = false; // Скрываем курсор
    SetConsoleCursorInfo(hConsole, &cursorInfo);

    // Позиции для каждого прогресс-бара
    vector<COORD> positions(threads_data.size());

    { // Область видимости мьютекса
        lock_guard<mutex> lock(cout_mutex);
        for (size_t i = 0; i < threads_data.size(); ++i) {
            cout << "Поток " << i + 1 << " (ID: " << threads_data[i].id << ") [";
            for (int j = 0; j < progress_bar_length; ++j) cout << " ";
            cout << "] 0%\n";
            positions[i] = { 0, static_cast<SHORT>(i) };
        }
    } // Тут мьютекс освобождается

    // Основной цикл обновления
    while (true) {
        bool all_finished = true;

        {
            lock_guard<mutex> lock(cout_mutex);
            for (size_t i = 0; i < threads_data.size(); ++i) {
                if (!threads_data[i].finished) all_finished = false;

                float progress = static_cast<float>(threads_data[i].progress) / progress_bar_length;
                int percentage = static_cast<int>(progress * 100);

                // Устанавливаем позицию курсора
                SetConsoleCursorPosition(hConsole, positions[i]);
                cout << "Поток " << i + 1 << " (ID: " << threads_data[i].id << ") [";

                int pos = progress_bar_length * progress;
                for (int j = 0; j < progress_bar_length; ++j) {
                    if (j < pos) cout << "=";
                    else if (j == pos) cout << ">";
                    else cout << " ";
                }

                if (threads_data[i].finished) {
                    cout << "] Завершен. Время: " << threads_data[i].duration.count() << " мс   ";
                }
                else {
                    cout << "] " << setw(3) << percentage << "%   ";
                }
            }
        }

        if (all_finished) break;
        this_thread::sleep_for(milliseconds(refresh_rate_ms));
    }

    // Восстанавливаем курсор
    cursorInfo.bVisible = true;
    SetConsoleCursorInfo(hConsole, &cursorInfo);
}

int main() {
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);

    const int num_threads = 5;

    vector<ThreadData> threads_data(num_threads);
    vector<thread> threads;

    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> length_dist(1000, 5000); // Случайная длительность от 1 до 5 секунд

    for (int i = 0; i < num_threads; ++i) {
        threads_data[i].task_length = length_dist(gen);
    }

    // Запускаем потоки с задачами
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back(calculate_task, i, ref(threads_data));
    }

    // Запускаем поток для отображения прогресса
    thread display_thread(display_progress, cref(threads_data));

    // Ждем завершения всех рабочих потоков
    for (auto& t : threads) {
        t.join();
    }

    // Ждем завершения потока отображения
    display_thread.join();

    cout << "\nВсе потоки завершили работу." << endl;
    return 0;
}