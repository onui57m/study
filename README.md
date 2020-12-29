# study
練習で書いたプログラムを置く場所

## life_game
[ライフゲーム](https://ja.wikipedia.org/wiki/%E3%83%A9%E3%82%A4%E3%83%95%E3%82%B2%E3%83%BC%E3%83%A0)

OpenGLの練習がしたかった。C++で実装。OpenGL用のコンパイルオプションは -lglut -lGLU -lGL

- 参考にしたページ
  - [GLUTによる「手抜き」OpenGL入門](https://tokoik.github.io/opengl/libglut.html)
  - [OpenGL入門](http://wisdom.sakura.ne.jp/system/opengl/index.html)

## Brusselator
[ブラッセレータ](https://en.wikipedia.org/wiki/Brusselator)

OpenGLの練習がしたかった（その2）。C++で実装。

拡散係数が大きいために時間刻み幅があまり大きくできないのでやや遅い。もっとDを小さくしても良いかも。

中立安定曲線のグラフの書き出しを隣のウィンドウで実施。

- 参考にしたページ
  - [反応拡散系（FitzHugh–Nagumo 方程式）](https://kmaeda.net/kmaeda/demo/rds-fn/)

## md
分子動力学シミュレーション。まだLJポテンシャルを入れただけ。

OpenGLの練習がしたかった（その3）。まだ微妙にビジュアライザがバグっている気がする

- そのうちやりたいこと
  - 統計情報の計算
  - エラーの処理（周期境界を2回超えたら怒る。近づきすぎたら怒る。カットオフと箱の大きさ）
  - book-keeping の実装 -> verlet listを実装した。リストのサイズが適正かのエラーを吐きたい。
  - 温度・体積・圧力の制御
  - マウスでクリックした位置を重心にする
  - グラフの表示（エネルギー、温度などの物理量を同時に表示したい）
  - マウス操作による視点移動
  - 画像の書き出し

- 参考にしたページ
  - [分子動力学法ステップ・バイ・ステップ その1](https://qiita.com/kaityo256/items/2356fff922938ae3c87c)
  - [床井研究室 - 第６回 視点の移動](http://marina.sys.wakayama-u.ac.jp/~tokoi/?date=20090902)
  - [ロドリゲスの回転公式の表現行列](http://w3e.kanazawa-it.ac.jp/math/physics/category/physical_math/linear_algebra/henkan-tex.cgi?target=/math/physics/category/physical_math/linear_algebra/rodrigues_rotation_matrix.html)

## fseek
fseekの仕様を知るためにテストした。良くわからない。
- 参考にしたページ
  - [fseek - cppreference.com](https://en.cppreference.com/w/c/io/fseek)
  - [9.5 ファイルの位置決め](https://docs.oracle.com/cd/E19957-01/805-7889/z4000016dc580/index.html)
  - [ファイル内の位置の再設定(C++ ライブラリ・リファレンス)](https://docs.oracle.com/cd/E19205-01/820-2985/loc_io/9_5.htm)
  - [C++でのバイナリファイルの入出力について](http://voidy21.hatenablog.jp/entry/20090119/1232388689)

## make_pollock
ビットマップ画像を受け取ってジャスティンポロック様画像に変換するプログラムを作成（予定）。自然の風がフラクタル性のある画を作るということで、風による作画を行う。

画像の取り扱い、機械学習の練習をしたい。

- 順次やりたいこと
  - スコア関数をいろいろ試す（機械学習による実装？GANによる作画とか？）

- 参考にしたページ
  - [Bitmapファイルフォーマット](http://www.umekkii.jp/data/computer/file_format/bitmap.cgi)

## Licenses
[MIT licenses](https://opensource.org/licenses/mit-license.php)
