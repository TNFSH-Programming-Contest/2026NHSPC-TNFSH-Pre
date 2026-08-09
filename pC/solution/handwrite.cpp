#include <stdio.h>

int ans[5] = {1, 2, 4, 8, 16};

int main(void) {
    int n;
    scanf("%d\n", &n);
    if (n > 5) return 0;

    printf("%d\n", ans[n - 1]);
}