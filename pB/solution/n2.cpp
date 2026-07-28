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

    printf("%d\n", min(abs(t1 - t2), t1 + 43200 - t2));

    return 0;
}