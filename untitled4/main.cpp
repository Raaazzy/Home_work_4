#include <iostream>
#include <pthread.h>
#include <string>
#include <unistd.h>
#include <vector>
#include <queue>

using namespace std;

#define SECTIONS_NUM 10

/**
 * Класс продавца
 */
class Island {
public:
    int id = 0;

    Island() {}
};

/**
 * Класс покупателя
 */
class Pirate {
public:
    int id = 0;

    std::queue<int> plan;
    Pirate() {}
};

// Буфер
vector<Pirate *> buf(SECTIONS_NUM);

bool flag = true;

unsigned int seed = 102;// инициализатор генератора случайных чисел

pthread_mutex_t mutex;// мьютекс для условных переменных

// поток-пират блокируется этой условной переменной,
// когда количество занятых ячеек становится равно 0
pthread_cond_t section_empty;

// Стартовая функция потоков – пиратов
void *PirateFunc(void *param) {
    Pirate *pirate = ((Pirate *) param);
    while (!pirate->plan.empty()) {
        // Извлечь элемент из буфера
        cout << "\nPirate: " + to_string(pirate->id) + " moved to the Island: " + to_string(pirate->plan.front()) + " \t\t\tclock: " + to_string(clock());
        sleep(1);
        // Заснуть, если нужный остров занят
        while (buf[pirate->plan.front()] != nullptr) {
        }
        // Защита операции чтения
        pthread_mutex_lock(&mutex);
        buf[pirate->plan.front()] = pirate;
        // Заснуть, если нужный остров занят
        while (buf[pirate->plan.front()] != nullptr) {
        }
        pirate->plan.pop();
        // Конец критической секции
        pthread_mutex_unlock(&mutex);
    }
    cout << "\nPirate: " + to_string(pirate->id) + " went away..." + " \t\t\t\t\tclock: " + to_string(clock());
    return nullptr;
}

//стартовая функция потоков – островов
void *IslandFunc(void *param) {
    Island island = *((Island *) param);
    while (flag) {
        if (buf[island.id] == nullptr) {
            continue;
        }
        cout << "\nIsland: " + to_string(island.id) + " IS being searched by pirate: " + to_string(buf[island.id]->id) + " \t\tclock: " + to_string((clock()));
        sleep(2);
        cout << "\nIsland: " + to_string(island.id) + " FINISHED search by pirate: " + to_string(buf[island.id]->id) + " \t\t\tclock: " + to_string((clock()));
        buf[island.id] = nullptr;
        // Разбудить потоки-пиратов после обновления элемента буфера
//        pthread_cond_broadcast(&section_empty);
    }
    cout << "\nIsland: " + to_string(island.id) + " has been searched... " + " \t\t\tclock: " + to_string((clock()));
    return nullptr;
}

int Task_random() {
    srand(clock());
    int num_sec = rand() % 8 + 3;
    printf("Generated number of sections: %lf\n", num_sec);
    return num_sec;
}

int main(int argc, char *argv[]) {
    int  section_num;
    cout << "\nPlease enter the number of islands (3 <= number <= 10):\n";
    cin >> section_num;
    if (section_num > 10 || section_num < 3) {
        cout << "\nYou entered incorrect data.\n";
        return 0;
    }

    std::vector<Island> islands(section_num);
    std::vector<pthread_t> threads_islands(islands.size());

    std::vector<Pirate> pirates(section_num - 1);
    std::vector<pthread_t> threads_pirates(pirates.size());

    srand(seed);
    int i;
    // Инициализация мутексов и семафоров
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&section_empty, NULL);
    cout << "\nThe store is opening!\n\n";

    // Запуск островов
    for (i = 0; i < islands.size(); i++) {
        // Зададим id для острова
        islands[i].id = i;
        // Запускаем поток
        pthread_create(&threads_islands[i], nullptr,
                       IslandFunc,
                       &islands[i]);
        cout << "\nIsland: " + to_string(islands[i].id) + " came to work";
    }

    // Запуск пиратов
    for (i = 0; i < pirates.size(); ++i) {
        pirates[i].id = i;
        // int num = rand() % 5;
        // Создаём случайный план для пирата
        queue<int> plan;
        string str = "\nPirate: " + to_string(pirates[i].id) + " come with plan:";
        for (int j = 0; j < section_num; ++j) {
            int val = rand() % section_num;
            plan.push(val);
            str += " " + to_string(val);
        }
        cout << str;
        pirates[i].plan = plan;
    }

    for (int i = 0; i < pirates.size(); ++i) {
        pthread_create(&threads_pirates[i], nullptr,
                       PirateFunc,
                       &pirates[i]);
    }

    // Ждём пока кончатся все пираты
    for (unsigned long long threads_pirate : threads_pirates) {
        pthread_join(threads_pirate, nullptr);
    }

    flag = false;

    // Ждём пока обыщут все острова
    for (unsigned long long threads_island : threads_islands) {
        pthread_join(threads_island, nullptr);
    }
    return 0;
}
