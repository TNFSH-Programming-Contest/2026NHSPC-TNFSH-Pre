#include <stdio.h>
#include <limits.h>

typedef long long loli;
int times[14];

loli min(loli a, loli b) {
    return a < b ? a : b;
}

int main(void) {
    // nitrogen

    int n;
    scanf("%d", &n);

    for (int i = 0; i < n; i++) {
        scanf("%d", &times[i]);
    }

    loli ans = LLONG_MAX;
    for (int target = 0; target < 43200; target++) {
        loli cost = 0;
        for (int i = 0; i < n; i++) {
            cost += (target - times[i] + 43200) % 43200;
        }
        ans = min(ans, cost);
    }

    printf("%lld\n", ans);
}