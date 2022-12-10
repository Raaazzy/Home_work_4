#include <iostream>
#include <pthread.h>
#include <string>
#include <unistd.h>
#include <vector>
#include <queue>

using namespace std;

#define SECTIONS_NUM 10

// Класс участков
class Island {
public:
    int id = 0;
    bool isThereTreasure = false;

    Island() {}
};


// Класс группы пиратов
class Pirate {
public:
    int id = 0;
    std::queue<int> plan;

    Pirate() {}
};

// Буфер
vector<Pirate *> buf(SECTIONS_NUM);

bool flag = true;

pthread_mutex_t mutex;// мьютекс для условных переменных

// поток-пират блокируется этой условной переменной,
// когда количество занятых ячеек становится равно 0
pthread_cond_t section_empty;

// Стартовая функция потоков – группы пиратов
void *PirateFunc(void *param) {
    Pirate *pirate = ((Pirate *) param);
    while (!pirate->plan.empty()) {
        // Извлечь элемент из буфера
        cout << "\nPirate: " + to_string(pirate->id) + " moved to the Island: " + to_string(pirate->plan.front()) +
                " \t\t\tclock: " + to_string(clock());
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
    // Проходимся по всем участкам
    while (flag) {
        // Если участки закончились, то выходим из цикла
        if (buf[island.id] == nullptr) {
            continue;
        }
        cout << "\nIsland: " + to_string(island.id) + " IS being searched by pirate: " + to_string(buf[island.id]->id) +
                " \t\tclock: " + to_string((clock()));
        sleep(2);
        cout << "\nIsland: " + to_string(island.id) + " FINISHED search by pirate: " + to_string(buf[island.id]->id) +
                " \t\t\tclock: " + to_string((clock()));
        buf[island.id] = nullptr;
        // Разбудить потоки-пиратов после обновления элемента буфера
    }
    // Проверяем есть ли сокровища на участке
    if (island.isThereTreasure) {
        cout << "\nIsland: " + to_string(island.id) + " has been searched... Pirates have found treasure here!" +
                " \t\t\tclock: " + to_string((clock()));
    } else {
        cout << "\nIsland: " + to_string(island.id) + " has been searched... The pirates didn't find anything here!" +
                " \t\t\tclock: " + to_string((clock()));
    }
    return nullptr;
}

int main(int argc, char *argv[]) {
    // Пользователь вводит количество участков, на которые разбит остров
    int section_num;
    cout << "\nPlease enter the number of islands (3 <= number <= 10):\n";
    cin >> section_num;
    if (section_num > 10 || section_num < 3) {
        cout << "\nYou entered incorrect data.\n";
        return 0;
    }
    // Записываем все id участков в vector
    std::vector<int> numbers(section_num);
    for (int i = 0; i < section_num; ++i) {
        numbers[i] = i;
    }

    // Создаем vector во всеми участками
    std::vector<Island> islands(section_num);
    std::vector<pthread_t> threads_islands(islands.size());

    // Создаем vector во всеми пиратами
    std::vector<Pirate> pirates(section_num - 1);
    std::vector<pthread_t> threads_pirates(pirates.size());

    srand(clock());
    int i;

    // Инициализация мутексов и семафоров
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&section_empty, NULL);
    cout << "\nStart searching!\n\n";

    // Запуск островов
    for (i = 0; i < islands.size(); i++) {
        // Задаем id для острова
        islands[i].id = i;

        // Задаем наличие сокровищ на острове
        int val = rand() % 2;
        if (val == 1) {
            islands[i].isThereTreasure = true;
        } else {
            islands[i].isThereTreasure = false;
        }
        // Запускаем поток
        pthread_create(&threads_islands[i], nullptr,
                       IslandFunc,
                       &islands[i]);
        cout << "\nIsland " + to_string(islands[i].id) + ";";
    }

    // Запуск пиратов
    for (i = 0; i < pirates.size(); ++i) {
        pirates[i].id = i;
        // Создаём случайный план для пирата, по которому он будет обыскивать участки
        queue<int> plan;
        string str = "\nA group of pirates " + to_string(pirates[i].id) + " will search the island ";
        if (i == pirates.size() - 1) {
            plan.push(numbers[0]);
            str += " " + to_string(numbers[0]);
            plan.push(numbers[1]);
            str += " " + to_string(numbers[1]);
            numbers = {};
        } else {
            int val = rand() % numbers.size();
            plan.push(numbers[val]);
            str += " " + to_string(numbers[val]);
            numbers.erase(numbers.begin() + val);
        }
        cout << str;
        pirates[i].plan = plan;
    }

    // Создаем потоки
    for (int i = 0; i < pirates.size(); ++i) {
        pthread_create(&threads_pirates[i], nullptr,
                       PirateFunc,
                       &pirates[i]);
    }

    // Ждём пока кончатся все пираты
    for (unsigned long long threads_pirate: threads_pirates) {
        pthread_join(threads_pirate, nullptr);
    }

    flag = false;

    // Ждём пока обыщут все острова
    for (unsigned long long threads_island: threads_islands) {
        pthread_join(threads_island, nullptr);
    }
    return 0;
}
