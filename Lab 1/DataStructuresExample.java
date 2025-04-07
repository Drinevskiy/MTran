import java.util.ArrayList;
import java.util.HashMap;

public class DataStructuresExample {
    // Функция для отображения элементов списка
    public static void displayList(ArrayList<String> list) {
        System.out.println("Элементы списка:");
        int length = list.size();
        for (int i = 0; i < length; ++i) {
            String item = list.get(i);
            System.out.println(item);
        }
    }

    public static void main(String[] args) {
        // ArrayList
        ArrayList<String> fruits = new ArrayList<>();
        fruits.add("Яблоко");
        fruits.add("Банан");
        fruits.add("Вишня");

        // HashMap
        HashMap<String, Integer> fruitPrices = new HashMap<>();
        fruitPrices.put("Яблоко", 50);
        fruitPrices.put("Банан", 30);
        fruitPrices.put("Вишня", 80);

        // Вызов функции
        displayList(fruits);

        // Вывод цен на фрукты
        System.out.println("Цены на фрукты:");
        
        // for (String fruit : fruitPrices.keySet()) {
        //     System.out.println(fruit + ": " + fruitPrices.get(fruit) + " руб.");
        // }
    }
}