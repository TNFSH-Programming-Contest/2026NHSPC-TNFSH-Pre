# 分蛋糕 4
$\color{red}\text{本題為互動題，需要實作兩個檔案。}$

南宮柳棲今天邀請了資訊社的大家參加他的派對。派對中有一個超大超大的蛋糕，從左到右編號為第 $0$ 段至第 $n-1$ 段。

為了追求公平，蛋糕必須被徹底切分，直到所有蛋糕都成為單獨的一小塊。對於任何一段連續的蛋糕區間 $[l,r]$，切蛋糕的人可以選擇一個位置 $k$（$l \le k < r$）切一刀，將其分成 $[l,k]$ 與 $[k+1,r]$ 兩塊，接著再分別將左右兩塊徹底切分。

對每個區間 $[l,r]$，都有一個事先決定的非負整數 $w_{l,r}$。每當對區間 $[l,r]$ 切下第一刀時，柳棲想要跳起來大喊「6767」的衝動值會增加 $w_{l,r}$。單獨一小塊不需要再切，因此 $w_{i,i}=0$。

舉例來說，如果選擇在位置 $k$ 切開 $[l,r]$，這次完整切分所產生的總衝動值為 $w_{l,r}+\operatorname{cost}(l,k)+\operatorname{cost}(k+1,r)$

其中 $\operatorname{cost}(x,y)$ 代表將區間 $[x,y]$ 徹底切分所需的最小總衝動值。

這份費用表 $w$ 雖然是事先決定的，但柳棲知道它完全反映了蛋糕的物理尺寸與視覺張力，並保證滿足兩個極為自然的規律：

 - \textbf{大就是好} : 蛋糕區間越長，視覺震撼越強。對於任何一段連續的蛋糕，如果只取其中一部分連續的子區間來切，該子區間產生的衝動值絕對不會超過原本整段大區間的衝動值。
 - \textbf{越長越好} : 蛋糕的視覺衝擊具有邊際遞增的效應。想像有兩段右邊界對齊、但左邊界不同的蛋糕區間（即一長一短）。如果將這兩段蛋糕的右邊界同時向右延伸一段相同的長度，那麼原本就比較長（左邊界較遠）的那段蛋糕，它所產生的「衝動值增加量」，一定大於等於較短蛋糕的衝動值增加量。

柳棲有著預知未來的能力，他知道完整的費用表 $w$。但是，在派對上實際負責切蛋糕的巴漆並不知道任何 $w_{l,r}$。

柳棲只能在派對開始前，傳送一段長度不超過 $B$ 個位元的二進位編碼給巴漆。派對開始後，現場會出現 $Q$ 個詢問。每次詢問會給定一個區間 $[l,r]$，巴漆必須回答：\textbf{如果只拿出區間 $[l,r]$ 的蛋糕進行徹底切分，為了讓總衝動值最小，第一刀需要切在哪個位置 $k$？}如果在多個位置切第一刀都能得到最小總衝動值，回答其中任意一個即可。

\clearpage

## 實作說明

本題是 **Two Step** 題型。你需要上傳兩個檔案。

### 柳棲

第一個檔案為 `sixseven.cpp`，你必須在該檔案內實作 `encode` 這個函式:

```cpp
std::string encode(int n, const std::array<std::array<long long, 2000>, 2000>& w);
```

 - `n` 是蛋糕的總段數。
 - `w` 是一個 $2000 \times 2000$ 的二維陣列。
 - 對所有 $0\le l\le r<n$，`w[l][r]` 等於 $w_{l,r}$。
 - 當 $l>r$ 時，`w[l][r]` 的內容沒有意義，請勿使用。
 - 對於所有 $n \le i,\ j < 2000$ 的 `w[i][j]`，其內容沒有意義，請勿使用。
 - 你必須回傳一個只包含字元 `0` 與 `1`、長度不超過 $B$ 的字串。

### 巴漆

第二個檔案為 `eightseven.cpp`，你必須在該檔案內實作 `decode` 與 `query` 兩個函式:

```cpp
void decode(int n, const std::string& S);
```

 - `n` 是蛋糕的總段數。
 - `S` 是第一個程式中 `encode` 回傳的二進位字串。
 - 第二個程式不會取得費用表 $w$。

```cpp
int query(int l, int r);
```

 - `l`、`r` 代表本次詢問的蛋糕區間 $[l,r]$。
 - 你必須回傳一個最佳的第一刀切點 $k$，且 $l \le k < r$。
 - 如果有多個最佳切點，回傳其中任意一個即可。

\clearpage

## 執行細節

 - 你上傳的兩個檔案會與評測端準備的 `stub.cpp` 一同編譯成一個執行檔 `Cake_4`，你可以在兩個檔案內宣告任何全域變數或內部函數，但請將這些函數宣告在匿名命名空間內以避免變數撞名，詳細請參考範例程式碼。
 - 評測端會額外準備 `manager.cpp` 來操作兩個行程之間的互動。具體來說，評測端在編譯出兩份執行檔 `manager` 和`Cake_4` 後，兩份執行檔將會同時運行，並用以下方式測試:
    1. 評測端建立兩個不同的行程 `sixseven` 與 `eightseven` (`sixseven` 與 `eightseven` 都是執行 `Cake_4` 得到) 以及 `manager` 行程
    2. `manager` 從輸入檔讀入輸入
    3. `manager` 將所需資料透過 `pipe` 傳給程式 `sixseven`，並呼叫 `encode()`
    4. `sixseven` 在收到 `encode()` 的回傳值後，將回傳值透過 `pipe` 傳給 `manager`
    5. `manager` 將所需資料透過 `pipe` 傳給程式 `eightseven` ，並呼叫 `decode()`
    6. 接著 `manager` 將會執行 $Q$ 次的詢問，每次呼叫 `query()`
    7. 每次詢問的時候，`eightseven` 收到 `query()` 的回傳值，將回傳值透過 `pipe` 回傳給 `manager` 並判斷結果
 - 評分時評測端會建立兩個不同且隔離的行程 (Process)，這代表兩分程式之間的全域變數或內部函數是不能共用的。你也不能使用任何方法來和任何外部檔案互動，任何對 stdout、stderr 的輸出會被忽略。
 - `manager.cpp` 跟 `stub.cpp` 的所花時間微小至可忽略不計。

## 測資限制

- $2\le n\le 2000$
- $0\le w_{l,r}\le 3\times 10^{15}$
- $w_{i,i}=0$
- 費用表 $w$ 滿足題目開頭所述的兩項性質
- $1\le Q\le 10^5$
- $B=4\times 10^6$
- 每次詢問皆滿足 $0\le l < r < n$

\clearpage

## 互動範例

考慮附件中第一筆範例測試資料的情境

$n = 4, Q = 3$

$$
w=\begin{pmatrix}
0&1&3&19\\
 &0&2&18\\
 & &0&16\\
 & & &0
\end{pmatrix}.
$$

一個可能被評為 `Accepted` 的互動例子顯示如下:

|評分程式端|參賽者端|
|:-----------|:-----------|
|呼叫`encode(4,W)`||
||回傳 $S$|
|呼叫 `decode(4, S)`||
|呼叫 `query(0, 3)`||
||回傳 $2$|
|呼叫 `query(0, 1)`||
||回傳 $0$ |
|呼叫 `query(1, 3)`||
||回傳 $2$|

- $W$ 的值為 `{{0,1,3,19},{87,0,2,18},{87,87,0,16},{87,87,87,0}}`
- $S$ 的值為 "100010101000010101010101111010100010"(我隨便亂打的)

## 子任務

\subtasks

\clearpage

## 範例評分程式

為了測試方便，範例評分程式不完全按照執行細節段落內所講的執行，而是會將你的程式編譯成單一執行檔執行後，直接在該次執行依序呼叫 `sixseven` 和 `eightseven` 兩支程式。請注意，在範例評分程式執行的過程中，`sixseven` 與 `eightseven` 將可以共享不在匿名空間內的全域變數或內部函數，\textbf{但這在正式評分程式是不允許的}，因此還請特別注意別誤共享了資訊。

請使用 `compile_cpp.sh` 或 `compile_cpp.bat` 來編譯你寫好的兩支程式。並使用 `run_cpp.sh` 或 `run_cpp.bat` 執行範例評分程式，它將採用以下格式輸入:\
\noindent\fbox{%
\begin{minipage}{\dimexpr\textwidth-2\fboxsep-2\fboxrule\relax}
\raggedright
$
\begin{array}{l}
    n \quad Q \\
    w_{0, 0}, w_{0, 1}, \ldots, w_{0, n-1} \\
    w_{1, 1}, w_{1, 2}, \ldots, w_{1, n-1} \\
    \vdots \\
    w_{n-1, n-1} \\
    l_{1}, r_{1} \\
    l_{2}, r_{2} \\
    \vdots \\
    l_{Q}, r_{Q}
\end{array}
$
\end{minipage}%
}

範例評分程式首先呼叫 `encode(n, w)`。接著，若範例評分程式偵測到 `encode()` 的回傳值有任何不合法，此程式將輸出\
\noindent\fbox{%
\begin{minipage}{\dimexpr\textwidth-2\fboxsep-2\fboxrule\relax}
\raggedright
$\text{wrong answer: msg}$
\end{minipage}%
}

後並終止程式執行，而 `msg` 為下列其中之一錯誤訊息。

 - `Length limit exceeded` : `encode()` 的回傳字串長度超過 $B$ 個字元。
 - `Invaild returned character` : `encode()` 的回傳字串不是一個 $01$ 字串。

否則，範例評分程式將會輸出一行\
\noindent\fbox{%
\begin{minipage}{\dimexpr\textwidth-2\fboxsep-2\fboxrule\relax}
\raggedright
$S$
\end{minipage}%
}

接著呼叫 `decode(n, S)`，其中的 $S$ 將會是呼叫 `encode()` 所拿到的回傳字串。

接著範例評分程式會依序執行 $Q$ 次詢問，每次呼叫 `query(l, r)`，並以以下格式印在標準輸出中:\
\noindent\fbox{%
\begin{minipage}{\dimexpr\textwidth-2\fboxsep-2\fboxrule\relax}
\raggedright
$
i \text{ : query}(l, r) \text{ } k
$
\end{minipage}%
}

 - $l, r$ 在第 $i$ 次詢問中為 $l_i, r_i$
 - $k$ 為 `query(l, r)` 所得到的回傳值

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
