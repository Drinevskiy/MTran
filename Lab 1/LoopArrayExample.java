public class LoopArrayExample {
    public static void main(String[] args) {
        // Массив целых чисел
        int[] numbers = {1, 2, 3, 4, 5}; 
        // Цикл for
        System.out.print("Числа в массиве: ");
        int length = 5;
        for (int i = 0; i < length; ++i) {
            System.out.print(numbers[i] + " ");
        }
        System.out.println();
        // Цикл do...while
        int index = 0;
        System.out.print("Числа в обратном порядке (do...while): ");
        int temp = length-1-index;
        do {
            temp = length-1-index;
            System.out.print(numbers[temp] + " ");
            ++index;
        } while (index < length);
        System.out.println();
        // Цикл while
        int sum = 0;
        int j = 0;
        while (j < length) {
            sum += numbers[j];
            continue;
        }
        // float i = 32.4f;
        // numbers[i] = 34;
        numbers[sum] = 12;
        numbers[j] = 34;
        System.out.println("Сумма чисел в массиве: " + sum);
    }
}