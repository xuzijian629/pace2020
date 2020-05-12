# Update History

### v1

- どうせ induced subgraph しか使わなそうなので hash をやめる, key を g.nodesのBITSETにして、main_memo の値を lb, ub, ans、sep_memo の値を k, seps とする
- sep_memo はサイズ sep_memo[g.nodes].k 以下の min-sep を全列挙しているので、list_exact(g, k) は、k <= sep_memo[g.nodes].k が成立したとき、sep_memo[g.nodes].seps を見てサイズk以下のもののみを返す（なお予め uniqueしたあと BITSET::count() でもソートしてあるのでサイズ k を超えたらbreakしているだけ）
- longest_path_lb を追加