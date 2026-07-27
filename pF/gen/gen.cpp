#include "testlib.h"
#include <bits/stdc++.h>
using namespace std;
void appendBlock(vector<int>& a, int base, int len, string mode) {
    if (len == 0) return;

    auto put = [&](int x) {
        a.push_back(base + x);
    };

    if (mode == "pairs") {
        for (int x = 0; x < len; x++) {
            put(x);
            put(x);
        }
    }
    else if (mode == "even-up") {
        for (int x = 0; x < len; x++) put(x);
        for (int x = len - 1; x >= 0; x--) put(x);
    }
    else if (mode == "even-down") {
        for (int x = len - 1; x >= 0; x--) put(x);
        for (int x = 0; x < len; x++) put(x);
    }
    else if (mode == "odd-up") {
        for (int x = 0; x < len - 1; x++) put(x);
        put(len - 1);
        for (int x = len - 2; x >= 0; x--) put(x);
        put(len - 1);
    }
    else if (mode == "random") {
        vector<int> b;
        for (int x = 0; x < len; x++) {
            b.push_back(base + x);
            b.push_back(base + x);
        }
        shuffle(b.begin(), b.end());
        a.insert(a.end(), b.begin(), b.end());
    }
}
int main(int argc, char* argv[]) {
    registerGen(argc, argv, 1);

    int n = opt<int>(1);
    string mode = opt<string>(2);

    vector<int> a;
    a.reserve(2 * n);

    if (mode == "random") {
        for (int x = 0; x < n; x++) {
            a.push_back(x);
            a.push_back(x);
        }
        shuffle(a.begin(), a.end());
    }
    else if (mode == "pairs") {
        // 0 0 1 1 2 2 ...
        for (int x = 0; x < n; x++) {
            a.push_back(x);
            a.push_back(x);
        }
    }
    else if (mode == "even-up") {
        // 0 1 ... n-1 n-1 ... 1 0
        for (int x = 0; x < n; x++) a.push_back(x);
        for (int x = n - 1; x >= 0; x--) a.push_back(x);
    }
    else if (mode == "even-down") {
        // n-1 ... 0 0 ... n-1
        for (int x = n - 1; x >= 0; x--) a.push_back(x);
        for (int x = 0; x < n; x++) a.push_back(x);
    }
    else if (mode == "odd-up") {
        // 0 1 ... n-2 n-1 n-2 ... 1 0 n-1
        for (int x = 0; x < n - 1; x++) a.push_back(x);
        a.push_back(n - 1);
        for (int x = n - 2; x >= 0; x--) a.push_back(x);
        a.push_back(n - 1);
    }
    else if (mode == "odd-down") {
        // n-1 ... 1 0 1 ... n-1 0
        for (int x = n - 1; x >= 1; x--) a.push_back(x);
        a.push_back(0);
        for (int x = 1; x < n; x++) a.push_back(x);
        a.push_back(0);
    }
    else if (mode == "mixed") {
        vector<string> types = {
            "pairs",
            "even-up",
            "even-down",
            "odd-up",
            "random"
        };

        int blockCount = (int)types.size();
        int base = 0;

        for (int i = 0; i < blockCount; i++) {
            int len = n / blockCount;
            if (i < n % blockCount) len++;

            appendBlock(a, base, len, types[i]);
            base += len;
        }
    }
    else {
        quitf(_fail, "unknown mode: %s", mode.c_str());
    }

    cout << n << '\n';
    println(a);
}
