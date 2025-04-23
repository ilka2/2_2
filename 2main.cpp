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
const int refresh_rate_ms = 100; // ������� ���������� (� �������������)

struct ThreadData {
    atomic<int> progress{ 0 };
    atomic<bool> finished{ false };
    milliseconds duration{ 0 };
    thread::id id;
    int task_length; // ������� ������������ ������ ��� ������� ������
};

void calculate_task(int thread_num, vector<ThreadData>& threads_data) {
    auto start_time = high_resolution_clock::now();
    threads_data[thread_num].id = this_thread::get_id();

    // ��������� ��������� ����� ��� ��������
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> delay_dist(10, 100); // ��������� �������� ����� ������

    for (int i = 0; i <= progress_bar_length; ++i) {
        threads_data[thread_num].progress = i;
        this_thread::sleep_for(milliseconds(delay_dist(gen)));
    }

    auto end_time = high_resolution_clock::now();
    threads_data[thread_num].duration = duration_cast<milliseconds>(end_time - start_time);
    threads_data[thread_num].finished = true;
}

void display_progress(const vector<ThreadData>& threads_data) {
    // �������� handle ������� ��� ����� �������� ����������
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hConsole, &cursorInfo);
    cursorInfo.bVisible = false; // �������� ������
    SetConsoleCursorInfo(hConsole, &cursorInfo);

    // ������� ��� ������� ��������-����
    vector<COORD> positions(threads_data.size());

    { // ������� ��������� ��������
        lock_guard<mutex> lock(cout_mutex);
        for (size_t i = 0; i < threads_data.size(); ++i) {
            cout << "����� " << i + 1 << " (ID: " << threads_data[i].id << ") [";
            for (int j = 0; j < progress_bar_length; ++j) cout << " ";
            cout << "] 0%\n";
            positions[i] = { 0, static_cast<SHORT>(i) };
        }
    } // ��� ������� �������������

    // �������� ���� ����������
    while (true) {
        bool all_finished = true;

        {
            lock_guard<mutex> lock(cout_mutex);
            for (size_t i = 0; i < threads_data.size(); ++i) {
                if (!threads_data[i].finished) all_finished = false;

                float progress = static_cast<float>(threads_data[i].progress) / progress_bar_length;
                int percentage = static_cast<int>(progress * 100);

                // ������������� ������� �������
                SetConsoleCursorPosition(hConsole, positions[i]);
                cout << "����� " << i + 1 << " (ID: " << threads_data[i].id << ") [";

                int pos = progress_bar_length * progress;
                for (int j = 0; j < progress_bar_length; ++j) {
                    if (j < pos) cout << "=";
                    else if (j == pos) cout << ">";
                    else cout << " ";
                }

                if (threads_data[i].finished) {
                    cout << "] ��������. �����: " << threads_data[i].duration.count() << " ��   ";
                }
                else {
                    cout << "] " << setw(3) << percentage << "%   ";
                }
            }
        }

        if (all_finished) break;
        this_thread::sleep_for(milliseconds(refresh_rate_ms));
    }

    // ��������������� ������
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
    uniform_int_distribution<> length_dist(1000, 5000); // ��������� ������������ �� 1 �� 5 ������

    for (int i = 0; i < num_threads; ++i) {
        threads_data[i].task_length = length_dist(gen);
    }

    // ��������� ������ � ��������
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back(calculate_task, i, ref(threads_data));
    }

    // ��������� ����� ��� ����������� ���������
    thread display_thread(display_progress, cref(threads_data));

    // ���� ���������� ���� ������� �������
    for (auto& t : threads) {
        t.join();
    }

    // ���� ���������� ������ �����������
    display_thread.join();

    cout << "\n��� ������ ��������� ������." << endl;
    return 0;
}