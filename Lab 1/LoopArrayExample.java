public class LoopArrayExample {
    public static void main(String[] args) {
        // Массив целых чисел
        int[] numbers = {1, 2, 3, 4, 5};

        // Цикл for
        System.out.print("Числа в массиве: ");
        for (int i = 0; i < numbers.length; i++) {
            System.out.print(numbers[i] + " ");
        }
        System.out.println();

        // Цикл do...while
        int index = 0;
        System.out.print("Числа в обратном порядке (do...while): ");
        do {
            System.out.print(numbers[numbers.length - 1 - index] + " ");
            index++;
        } while (index < numbers.length);
        System.out.println();

        // Цикл while
        int sum = 0;
        int j = 0;
        while (j < numbers.length) {
            sum += numbers[j];
            j++;
        }
        System.out.println("Сумма чисел в массиве: " + sum);
    }
}