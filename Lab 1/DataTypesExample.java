public class DataTypesExample {
    public static void main(String[] args) {
        // Примитивные типы данных
        int age = 25;
        double salary = 55000.50;
        char grade = 'A';
        boolean isEmployed = true;

        // Условный оператор if
        if (isEmployed) {
            System.out.println("Возраст: " + age);
            System.out.println("Зарплата: $" + salary);
            System.out.println("Оценка: " + grade);
        } else {
            System.out.println("Не трудоустроен");
        }

        // Оператор switch case для оценки
        switch (grade) {
            case 'A':
                System.out.println("Отличная работа!");
                break;
            case 'F':
                System.out.println("Неудовлетворительно.");
                break;
            default:
                System.out.println("Недоступная оценка.");
                break;
        }
    }
}