#include <bits/stdc++.h>
using namespace std;

/* =========================================================
 * 你可以調的參數
 * ========================================================= */

#define HASH_BITS 16

/*
    BLOCK 越大：
      hash 數越少
      enumeration 越恐怖

    BLOCK=4 適合先測試 correctness / collision。
*/
constexpr int BLOCK = 4;

constexpr int MAXN = 2000;


/* =========================================================
 * Hash
 * ========================================================= */

using u64 = uint64_t;

static constexpr u64 BASE =
    0x9e3779b185ebca87ULL;

u64 splitmix64(u64 x) {
    x += 0x9e3779b97f4a7c15ULL;
    x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ULL;
    x = (x ^ (x >> 27)) * 0x94d049bb133111ebULL;
    return x ^ (x >> 31);
}


#if HASH_BITS == 64

using Hash = uint64_t;

Hash hash_step(Hash h, int x) {
    return h * BASE + (uint64_t)(x + 1);
}

#elif HASH_BITS == 32

using Hash = uint32_t;

Hash hash_step(Hash h, int x) {
    return (uint32_t)splitmix64(
        (uint64_t)h * 0x9e3779b1ULL +
        (uint64_t)(x + 1)
    );
}

#elif HASH_BITS == 16

using Hash = uint16_t;

Hash hash_step(Hash h, int x) {
    return (uint16_t)splitmix64(
        (uint64_t)h * 0x9e37ULL +
        (uint64_t)(x + 1)
    );
}

#endif


Hash calc_hash(const int *a, int len) {
    Hash h = 0;

    for (int i = 0; i < len; ++i)
        h = hash_step(h, a[i]);

    return h;
}


/* =========================================================
 * DP
 * ========================================================= */

static long long dp[MAXN][MAXN];
static uint16_t optv[MAXN][MAXN];

static int N;


/* =========================================================
 * Stored block
 * ========================================================= */

struct Block {
    Hash h;
};

static vector<Block> blocks;


/*
    row_begin[l]
    = 這一 row 的第一個 block 在 blocks 中的位置
*/
static int row_begin[MAXN];


/*
    decode cache

    recovered[l][r]
*/
static uint16_t recovered[MAXN][MAXN];


/*
    已經 decode 到哪裡
*/
static int decoded_until[MAXN];


/* =========================================================
 * Bit IO
 * ========================================================= */

struct BitWriter {
    string s;

    void put(bool b) {
        s.push_back(b ? '1' : '0');
    }

    void put_uint(uint64_t x, int bits) {
        for (int i = 0; i < bits; ++i)
            put((x >> i) & 1ULL);
    }
};


struct BitReader {
    const string &s;
    size_t p = 0;

    BitReader(const string &s_)
        : s(s_) {}

    bool get() {
        if (p >= s.size())
            return false;

        return s[p++] == '1';
    }

    uint64_t get_uint(int bits) {
        uint64_t x = 0;

        for (int i = 0; i < bits; ++i) {
            if (get())
                x |= (1ULL << i);
        }

        return x;
    }
};


/* =========================================================
 * DP
 * ========================================================= */

void build_dp(
    int n,
    const array<array<long long, 2000>, 2000>& w
) {
    for (int i = 0; i < n; ++i) {
        dp[i][i] = 0;
        optv[i][i] = i;
    }

    for (int len = 2; len <= n; ++len) {

        for (int l = 0; l + len <= n; ++l) {

            int r = l + len - 1;

            int lo = optv[l][r - 1];
            int hi = optv[l + 1][r];

            lo = max(lo, l);
            hi = min(hi, r - 1);

            long long best = LLONG_MAX;
            int bestk = lo;

            for (int k = lo; k <= hi; ++k) {

                long long cur =
                    dp[l][k] +
                    dp[k + 1][r] +
                    w[l][r];

                if (cur < best) {
                    best = cur;
                    bestk = k;
                }
            }

            dp[l][r] = best;
            optv[l][r] = bestk;
        }
    }
}


/* =========================================================
 * Block utilities
 * ========================================================= */

int get_block_L(int l, int block_id) {
    return l + 1 + block_id * BLOCK;
}


int get_block_len(int l, int block_id) {
    int L = get_block_L(l, block_id);

    return min(
        BLOCK,
        N - L
    );
}


int get_block_id(int l, int r) {
    /*
        r >= l+1
    */
    return (r - (l + 1)) / BLOCK;
}


/* =========================================================
 * encode
 * ========================================================= */

string encode(
    int n,
    const array<array<long long, 2000>, 2000>& w
) {
    N = n;

    build_dp(n, w);

    blocks.clear();

    int total_blocks = 0;

    for (int l = 0; l < n; ++l) {

        row_begin[l] = total_blocks;

        for (
            int L = l + 1;
            L < n;
            L += BLOCK
        ) {

            int len = min(
                BLOCK,
                n - L
            );

            int a[BLOCK];

            for (int i = 0; i < len; ++i)
                a[i] = optv[l][L + i];

            Block b;

            b.h = calc_hash(a, len);

            blocks.push_back(b);

            ++total_blocks;
        }
    }


    /*
        Header:
          n         : 11 bits
          block cnt : 22 bits

        22 bits 足夠裝最多約 1.999m 個 block。
    */

    BitWriter bw;

    bw.put_uint(n, 11);
    bw.put_uint(
        (uint64_t)blocks.size(),
        22
    );

    for (const Block &b : blocks) {

#if HASH_BITS == 64

        bw.put_uint(
            (uint64_t)b.h,
            64
        );

#elif HASH_BITS == 32

        bw.put_uint(
            (uint32_t)b.h,
            32
        );

#elif HASH_BITS == 16

        bw.put_uint(
            (uint16_t)b.h,
            16
        );

#endif
    }

    return bw.s;
}


/* =========================================================
 * decode
 * ========================================================= */

void decode(
    int n,
    const string& S
) {
    N = n;

    BitReader br(S);

    int encoded_n =
        (int)br.get_uint(11);

    int cnt =
        (int)br.get_uint(22);

    /*
        防止 malformed S 直接越界。
    */

    if (encoded_n != n)
        return;

    blocks.clear();
    blocks.resize(cnt);

    int id = 0;

    for (int l = 0; l < n; ++l) {

        row_begin[l] = id;

        int cnt_row =
            (n - l - 1 + BLOCK - 1)
            / BLOCK;

        for (int b = 0;
             b < cnt_row;
             ++b) {

            if (id >= cnt)
                return;

#if HASH_BITS == 64

            blocks[id].h =
                (uint64_t)br.get_uint(64);

#elif HASH_BITS == 32

            blocks[id].h =
                (uint32_t)br.get_uint(32);

#elif HASH_BITS == 16

            blocks[id].h =
                (uint16_t)br.get_uint(16);

#endif

            ++id;
        }

        /*
            [l,l+1] 的 opt 一定是 l。
        */
        if (l + 1 < n) {
            recovered[l][l + 1] = l;
            decoded_until[l] = l + 1;
        } else {
            decoded_until[l] = l;
        }
    }
}


/* =========================================================
 * Composition enumeration
 *
 * 我們知道：
 *
 *   a[0] <= a[1] <= ...
 *
 * 固定前一個值 prev。
 *
 * 設：
 *
 *   d[i] = a[i] - a[i-1] >= 0
 *
 * 不再使用 MAX_DELTA。
 *
 * ========================================================= */

bool enum_block_dfs(
    const Block &b,
    int len,
    int pos,
    int prev,
    int *a,
    int *answer
) {
    if (pos == len) {
        if (calc_hash(a, len) == b.h) {
            for (int i = 0; i < len; ++i)
                answer[i] = a[i];

            return true;
        }
        return false;
    }

    for (int x = prev; x <= N - 2; ++x) {
        a[pos] = x;

        if (enum_block_dfs(
                b,
                len,
                pos + 1,
                x,
                a,
                answer
            )) {
            return true;
        }
    }

    return false;
}

/* =========================================================
 * Decode one block
 * ========================================================= */

bool decode_block(
    int l,
    int block_id
) {
    int L =
        get_block_L(l, block_id);

    int len =
        get_block_len(l, block_id);

    int global_id =
        row_begin[l] + block_id;

    if (
        global_id < 0 ||
        global_id >= (int)blocks.size()
    )
        return false;

    const Block &b =
        blocks[global_id];

    int prev;

    if (L == l + 1)
        prev = l;
    else
        prev = recovered[l][L - 1];


    int a[BLOCK];
    int ans[BLOCK];

    /*
        特殊情況：
          len == 0 不會發生。
    */

    if (
    enum_block_dfs(
        b,
        len,
        0,
        prev,
        a,
        ans
    )
) {

        for (int i = 0; i < len; ++i) {

            int r = L + i;

            recovered[l][r] =
                (uint16_t)ans[i];
        }

        decoded_until[l] =
            L + len - 1;

        return true;
    }

    return false;
}


/* =========================================================
 * query
 * ========================================================= */

int query(
    int l,
    int r
) {
    if (l == r)
        return l;


    /*
        如果這一 row 已經解到 r，
        直接回答。
    */

    if (
        decoded_until[l] >= r
    )
        return recovered[l][r];


    /*
        一個 block 一個 block 往右解。

        不做 recursive query，
        避免之前版本的巨大重複成本。
    */

    int first =
        get_block_id(
            l,
            decoded_until[l] + 1
        );

    int last =
        get_block_id(
            l,
            r
        );

    for (
        int b = first;
        b <= last;
        ++b
    ) {

        if (
            !decode_block(
                l,
                b
            )
        ) {

            /*
                hash 沒找到：

                  - hash collision
                  - S 壞掉
                  - 或 enumeration 太大而實際上
                    無法完成

                實驗版回傳目前已知值，
                正式測試可改成 return -1。
            */

            if (
                decoded_until[l] >= l + 1
            )
                return recovered[l][
                    min(
                        r,
                        decoded_until[l]
                    )
                ];

            return l;
        }
    }

    return recovered[l][r];
}