# PACE2020
[Challenge description](https://pacechallenge.org/2020/td/)

# TestCase
exact, heuristicを含め、すべてのテストケースは `data/` 以下にあります。
自分でランダムグラフなどのケースを追加してもらっても構いません。

exactのデータについては [公式サイト](https://pacechallenge.org/2020/visualize.html) にビジュアライズされています。

## Input format
入力のグラフは以下の形式で与えられます。

```
p tdp n m
u_1 v_1
...
u_m v_m
```

## Output format
根付き木を出力します。

```
n
p_1
...
p_n
```

p_i はノード i の親です。 i が根の場合は代わりに 0 を出力します。

# プログラムの形式
* プログラムは上述したフォーマットに従って、グラフのデータを標準入力から受け取り、treedepth decompositionを標準出力に出力することを想定しています。

* プログラムは `src/` 以下のディレクトリに自由な形で記載することができますが、 `src/` 以下で `make` コマンドを叩いたときに、 `main` という実行ファイルが `src/` 直下に生成されるようにしてください。

# ベンチマーク
* `benchmark/run.sh` を実行すると、 `benchmark/config.sh` の内容に従って各テストケースを実行します。
* 実行結果は `solutions/` 以下に保存されます。
* 既存の解よりもよい解が求まった場合、 `solutions/best/` 以下が更新されます。
