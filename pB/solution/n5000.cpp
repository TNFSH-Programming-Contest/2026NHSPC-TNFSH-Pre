#include <stdio.h>
#include <limits.h>

typedef long long loli;

loli min(loli a, loli b) {
    return a < b ? a : b;
}

int main(void) {
    // nitrogen

    int n;
    scanf("%d", &n);
    int *times = new int[n];

    for (int i = 0; i < n; i++) {
        scanf("%d", &times[i]);
    }
    int all_same = 1;
    for (int i = 1; i < n; ++i) {
        if (times[i] != times[0]) all_same = 0;
    }
    if (all_same) {
        puts("0");
        return 0;
    }

    loli ans = LLONG_MAX;
    for (int i = 0; i < n; i++) {
        loli target = times[i];
        loli cost = 0;

        for (int j = 0; j < n; j++) {
            cost += (target - times[j] + 43200) % 43200;
        }

        ans = min(ans, cost);
    }

    printf("%lld\n", ans);
}