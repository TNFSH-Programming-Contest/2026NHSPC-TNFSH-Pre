#include <stdio.h>
#include <limits.h>

typedef long long loli;
int times[2005];

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