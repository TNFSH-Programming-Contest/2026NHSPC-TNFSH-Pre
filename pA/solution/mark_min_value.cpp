#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <stdbool.h>

int main(void) {
    int n, k;
    scanf("%d %d", &n, &k);

    long long *v = (long long*)malloc(n * sizeof(long long));
    bool *vis = (bool*)calloc(n, sizeof(bool));
    for (int i = 0; i < n; ++i) {
        scanf("%lld", &v[i]);
    }

    int find_count = 0;
    long long ans;

    while (find_count < k) {
        long long min = LLONG_MAX;
        size_t min_index = 0;
        for (size_t i = 0; i < (size_t)n; ++i) {
            if (!vis[i] && v[i] < min) {
                min = v[i];
                min_index = i;
            }
        }
        vis[min_index] = true;
        find_count++;
        ans = min;
    }


    printf("%lld\n", ans);
    free(v);
    free(vis);
}
