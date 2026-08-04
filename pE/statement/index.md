# 分蛋糕 4

南宮柳棲今天邀請了資訊社的大家參加他的派對。派對中有一個超大超大的蛋糕，從左到右編號為第 $0$ 段至第 $n-1$ 段。

為了追求公平，蛋糕必須被徹底切分，直到所有蛋糕都成為單獨的一小塊。對於任何一段連續的蛋糕區間 $[l,r]$，切蛋糕的人可以選擇一個位置 $k$（$l \le k < r$）切一刀，將其分成 $[l,k]$ 與 $[k+1,r]$ 兩塊，接著再分別將左右兩塊徹底切分。

對每個區間 $[l,r]$，都有一個事先決定的非負整數 $w_{l,r}$。每當對區間 $[l,r]$ 切下第一刀時，柳棲想要跳起來大喊「6767」的衝動值會增加 $w_{l,r}$。單獨一小塊不需要再切，因此 $w_{i,i}=0$。

舉例來說，如果選擇在位置 $k$ 切開 $[l,r]$，這次完整切分所產生的總衝動值為

$$
w_{l,r}+\operatorname{cost}(l,k)+\operatorname{cost}(k+1,r),
$$

其中 $\operatorname{cost}(x,y)$ 代表將區間 $[x,y]$ 徹底切分所需的最小總衝動值。

費用表 $w$ 保證具有以下性質。對任意 $0\le a\le b\le c\le d<n$：

$$w_{b,c}\le w_{a,d},$$

以及

$$w_{a,c}+w_{b,d}\le w_{a,d}+w_{b,c}.$$

柳棲有著預知未來的能力，他知道完整的費用表 $w$。但是，在派對上實際負責切蛋糕的巴漆並不知道任何 $w_{l,r}$。

柳棲只能在派對開始前，傳送一段長度不超過 $B$ 個位元的二進位編碼給巴漆。派對開始後，現場會出現 $Q$ 個詢問。每次詢問會給定一個區間 $[l,r]$，巴漆必須回答：**如果只拿出區間 $[l,r]$ 的蛋糕進行徹底切分，為了讓總衝動值最小，第一刀可以切在哪個位置 $k$？**

如果有多個位置都能得到最小總衝動值，回答其中任意一個即可。

\clearpage

## 互動說明

本題是 **Communication** 題型。你需要上傳兩份程式碼 `sixseven.cpp` 與 `eightseven.cpp`，且不需要實作 `main` 函式。

您必須在程式碼中使用 `#include "Cake_4.h"` 引入評測端提供的函式宣告。

你的程式碼會與評測程式提供的 `stub.cpp` 編譯成一個執行檔。評測時，同一個執行檔會在兩個不同的程式中各執行一次。兩個程式只能透過評測程式傳遞下述二進位字串。

你需要實作以下三個函式。

### 階段一：柳棲的編碼

你必須要在 `sixseven.cpp` 內實作以下函式，且評測程式會呼叫 `sixseven.cpp` 中的 `encode` 函式一次
```cpp
std::string encode(int n, std::vector<std::vector<long long>> w);
```

- `n` 是蛋糕的總段數。
- `w` 是一個 $n\times n$ 的二維陣列。
- 對所有 $0\le l\le r<n$，`w[l][r]` 等於 $w_{l,r}$。
- 當 $l>r$ 時，`w[l][r]` 的內容沒有意義，請勿使用。
- 你必須回傳一個只包含字元 `0` 與 `1`、長度不超過 $B$ 的字串。

### 階段二：巴漆的解碼與處理詢問

你必須要在 `eightseven.cpp` 內實作以下函式，\
且評測程式會將 `encode` 的回傳值傳給第二個程式並呼叫 `eightseven.cpp` 中的 `decode` 函式一次

```cpp
void decode(int n, std::string S);
```

- `n` 是蛋糕的總段數。
- `S` 是第一個程式中 `encode` 回傳的二進位字串。
- 第二個程式不會取得費用表 $w$。

接著，評測程式會呼叫 `eightseven.cpp` 中的 `query` 函式 $Q$ 次：

```cpp
int query(int l, int r);
```

- `l`、`r` 代表本次詢問的蛋糕區間 $[l,r]$。
- 你必須回傳一個最佳的第一刀切點 $k$，且 $l\le k<r$。
- 如果有多個最佳切點，回傳其中任意一個即可。

評測程式不會同時呼叫上述函式。第一個程式只會呼叫 `encode` 函式；第二個程式只會呼叫 `decode` 與 `query` 函式。

\clearpage

## 評分標準

如果 `encode` 回傳的字串包含 `0`、`1` 以外的字元，或長度超過 $B$，你會得到 **Wrong Answer (1)**。

如果 `query` 回傳的切點不滿足 $l \le k < r$，你會得到 **Wrong Answer (2)**。

如果 `query` 回傳的切點不是最佳切點，你會得到 **Wrong Answer (3)**。

如果你的程式不正常結束、超過限制或無法完成通訊，將得到相對應的評測結果。

## 輸入限制

- $2\le n\le 2000$
- $0\le w_{l,r}\le 3\times 10^{16}$
- $w_{i,i}=0$
- 費用表 $w$ 滿足題目開頭所述的兩項性質
- $1\le Q\le 10^5$
- $B=4\times 10^6$
- 每次詢問皆滿足 $0\le l<r<n$

## 子任務

\subtasks

\clearpage

## 呼叫範例

假設 $n=4$，且柳棲得到的矩陣上三角為

$$
w=\begin{pmatrix}
0&1&3&19\\
 &0&2&18\\
 & &0&16\\
 & & &0
\end{pmatrix}.
$$

若詢問依序為 $[0,3]$、$[0,1]$、$[1,3]$，`query` 可以依序回傳 `2`、`0`、`2`。`encode` 實際回傳的二進位字串則由你的實作決定。

<!--
內部／本機測試方式（此段不顯示於正式題面）：

1. 在倉庫根目錄啟動 TPS 環境：
   docker run --rm -it -v ./:/directory tobiichi3227/tps-github-runner bash
2. 進入題目目錄並產生測資：
   cd /directory/pE
   tps gen
3. 以正式 Communication manager、stub 與兩個程式執行解答：
   tps invoke solution/correct.cpp
4. 執行特定測資：
   tps invoke solution/correct.cpp -t 0-01

請勿直接執行 solution/correct.cpp 編譯出的程式；正式流程需要由 TPS
建立兩組 FIFO、啟動同一個 contestant executable 兩次，並同時啟動 manager。
-->
