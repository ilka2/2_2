#include <iostream>
#include <windows.h>

using namespace std;


class smart_array
{
public:
    int len;
    int nowLen;
    int* arr;

    smart_array(int len)
    {
        this->len = len;
        nowLen = 0;
        arr = new int[len]();
    }

    ~smart_array()
    {
        delete[] arr;
    }

    smart_array& operator=(const smart_array& s_array)
    {
        if (&s_array != this)
        {
            delete[] arr;
            arr = new int[s_array.len]();

            for (int i = 0; i < s_array.len; i++)
            {
                arr[i] = s_array.arr[i];
            }

            len = s_array.len;
            nowLen = s_array.nowLen;
        }

        return *this;
    }

    /*void expansione_array(int*& arr)
    {
        len *= 1.5f;

        int* array = new int[len]();

        for (int i = 0; i < sizeof(arr) / sizeof(arr[0]); i++)
        {
            array[i] = arr[i];
        }

        delete[] arr;
        arr = new int[len]();

        for (int i = 0; i < len; i++)
        {
            arr[i] = array[i];
        }

        delete[] array;
    }*/

    int* expansione_array(static int* arr)
    {
        len *= 2;

        int* array = new int[len]();

        for (int i = 0; i < (len / 2); i++)
        {
            array[i] = arr[i];
        }

        delete[] arr;

        return array;
    }

    void add_element(int number)
    {
        if (len == nowLen)
        {
            arr = expansione_array(arr);
        }

        arr[nowLen] = number;
        nowLen += 1;
    }


    int get_element(int number)
    {
        if (nowLen > number)
        {
            return arr[number];
        }
        throw runtime_error("Такого элемента массива еще не существует!");

        return 0;
    }

    void now_length()
    {
        cout << nowLen;
    }

    void length()
    {
        cout << len;
    }
};

int main()
{
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);

    try {
        smart_array arr(5);
        arr.add_element(1);
        arr.add_element(4);
        arr.add_element(155);

        smart_array new_array(2);
        new_array.add_element(44);
        new_array.add_element(34);

        arr = new_array;

        cout << arr.get_element(1) << endl;
        cout << new_array.get_element(1) << endl;
    }
    catch (const exception& ex) {
        cout << ex.what() << endl;
    }

    return 0;
}