#include <iostream>
#include <string>

int square(int n)
{
    return (n * n);
}
int main(int argc, char* argv[])
{
    int age = 25;
    double salary = 55000.50;
    char grade = 'A';
    bool isEmployed = true;
    int sq_age = square(age);
    if (isEmployed) {
        std::cout << "Возраст: " << age << std::endl;
        std::cout << "Зарплата: $" << salary << std::endl;
        std::cout << "Оценка: " << grade << "баллов" << std::endl;
    }
    else {
        std::cout << "Не трудоустроен" << std::endl;
    }
    switch (grade) {
    case 'A':
        switch (age) {
        case 18:
            std::cout << "Взрослый" << std::endl;
            break;
        case 15:
            std::cout << "Молодой" << std::endl;
            break;
        default:
            std::cout << "Возраст не определен" << std::endl;
            break;
        }
        std::cout << "Отличная работа!" << std::endl;
        break;
    case 'F':
        std::cout << "Неудовлетворительно." << std::endl;
        break;
    default:
        std::cout << "Недоступная оценка." << std::endl;
        break;
    }
}
