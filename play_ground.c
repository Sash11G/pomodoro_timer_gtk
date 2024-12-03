#include <stdio.h>

void count_calls() {
    int call_count_1 = 0;
    static int call_count_2 = 0;
    call_count_2++, call_count_1++;
    printf("Function called %d times\n", call_count_2);
    printf("Function called %d times\n", call_count_1);
}

int main() {
    count_calls(); // Output: Function called 1 times
    count_calls(); // Output: Function called 2 times
    count_calls(); // Output: Function called 3 times
    return 0;
}
