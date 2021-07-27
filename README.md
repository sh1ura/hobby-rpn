# ホビーRPN電卓 ソフトウェア，ケース

ここで提供するのは、<a href="https://github.com/tokyoff/hobby-rpn">TOKYO FLIP-FLOP「ホビーRPN電卓」</a>に関する以下の４点です。詳細はそれぞれのフォルダ内を見てください。

* **基本** <a href="https://github.com/ypsilon-takai/hobby-rpn">Ypsilon project版</a>のOLED表示ライブラリを Adafruit から u8g に変更したもの（hobby-rpn.ino)。Ypsilon project版が Adafruit + 独自フォント実装の方向に進化していますので fork しました。

* **関数電卓** 上記に関数電卓機能を加えたもの。ファンクションキーが表示され、三角関数や対数・指数関数が利用できます。

* **誤差なし計算版** 分数ですべての計算を行い誤差が生じない電卓（ウェブブラウザ版や Android アプリは<a href="https://shiura.com/html5/index.html">こちら</a>）のコードを移植したバージョン(exactRPN.ino)。こちらのソースコードはほとんどオリジナルですが、<a href="https://github.com/tokyoff/hobby-rpn">Tokyo Flip-flopさんのオリジナル</a>のキースキャン部分のソースコードを用いているのでforkしました。

* **ケース** 3Dプリンタで出力できるケースの3Dデータ。

![case1](https://user-images.githubusercontent.com/86639425/126858002-d30cc380-30f6-459b-919d-26c7947bd335.jpg)

## ホビーRPN電卓について

Tokyo Flip-flopさんが提供している小型のRPN電卓同人ハードです。
本家サイト：https://www.hobby-rpn.com/

