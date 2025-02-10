public class TestLexer {
    public static void main(String[] args) {
        // Пример переменных
        byte byte1 = 109; 
        short short1 = -129&00;
        int number1 = 42;
        int number2 = -12;
        long long1 = 123212ф1252;
        String text = "Hello, World!";
        char letter = 'A';
        boolean isValid = true;
        float fl1 = 3.4f;
        float fl2 = -3.41F;
        double dbl7 = -3.422d;
        double dbl3 = -3.422.5d;
        double dbl6 = -3.4225D;
        double dbl4 = 3о9.45ee;
        double dbl1 = 4e-2;
        double dbl5 = 4e;
        double dbl@2 = 4E+17;

        // Условие
        if (isValid) {
            System.out.println(text);
        } else {
            System.out.println("Invalid");
        }

        // Цикл
        for (int i = 0; i < number1; i++) {
            System.out.println(i);
        }
        number1++;
        ++number2;
        ++4;
        --number1;
        number1--;
        // Массив
        int[] array = {1, 2, 3, 4, 5};
        for (int num : array) {
            System.out.println(num);
        }
        
        // Класс
        class InnerClass {
            void display() {
                System.out.println("Inside Inner Class");
            }
        }
        hg543
        // Создание объекта
        InnerClass inner = new InnerClass();
        inner.display();
    }
}