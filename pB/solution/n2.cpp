#include <stdio.h>

int min(int a, int b) {
    return a < b ? a : b;
}

int abs(int a) {
    return a < 0 ? -a : a;
}

int main(void) {
    // nitrogen

    int n;
    scanf("%d", &n);
    int t1, t2;
    scanf("%d %d", &t1, &t2);

    int difference = abs(t1 - t2);
    printf("%d\n", min(difference, 43200 - difference));

    return 0;
}