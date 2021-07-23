# ホビーRPN電卓 (u8g版, 誤差なし分数計算版)

TOKYO FLIP-FLOP「ホビーRPN電卓」のファームウェアのうち <a href="https://github.com/ypsilon-takai/hobby-rpn">Ypsilon project版</a>のOLED表示ライブラリを Adafruit から u8g に変更したものです。

また、分数ですべての計算を行い誤差が生じない電卓（ウェブブラウザ版や Android アプリは<a href="https://shiura.com/html5/index.html">こちら</a>）のコードを移植したバージョンも exactRPN.ico としてアップしました。

前者については、Ypsilon project版が Adafruit + 独自フォント実装の方向に進化していますので fork しました。

後者はほとんどオリジナルですが、<a href="https://github.com/tokyoff/hobby-rpn">Tokyo Flip-flopさんのオリジナル</a>のキースキャン部分のソースコードを用いているのでforkとします。

## ホビーRPN電卓について

Tokyo Flip-flopさんが提供している小型のRPN電卓同人ハードです。
本家サイト：https://www.hobby-rpn.com/

## 変更内容（u8g 版：Ypsilon project版からの差分）

### u8glib を使うことでフォントをきれいに

Adafruit は小さいフォントの拡大になってしまうので、u8g の 10x20 フォントを使用しました。

<img src="https://user-images.githubusercontent.com/86639425/126626266-2c95b7e6-e6a9-4b4c-96d7-3dc33c8e471a.jpg" width="500">

### 表示変更（桁数、右寄せ）

文字の幅が小さくなったので表示桁数も12桁に増やしました。

また、数値は右寄せのほうが見やすいと思ったので右寄せのコードを追加しました。

### - 長押しで、符号反転の動作を戻した

マイナス - 長押しのあとで数値入力すると文字がつながってしまうので、確定動作に戻しました。

好みかなという気はします。

### リセット時のプロンプトを > にした

リセット後やスタックを消去したときの 0. に違和感があったのでプロンプト的に > を表示するようにしました。

## 誤差なし分数計算版（exactRPN.ico）

<img src="https://user-images.githubusercontent.com/86639425/126646046-5edf0f8c-1d34-4820-ae6a-0b84fbc2fc58.jpg" width="500">

操作はよく似ていますが、以下の点で異なります。

### '*' 長押しで小数表示・分数表示の切り替え

かわりに平方根の計算はできません（無理数は誤差なし計算できないため）

### HP RPL 系の電卓に近い挙動

HP伝統の、Enter を推すと x と y に同じ数値が入る挙動が個人的には嫌いなため，HP28S以降50Gまで用いられているタイプ（Enter を押すと確定されるだけ）にしています。

### その他の操作

- 確定後 Enter を更に押すとスタックトップが複製されます (DUP)
- '+' の長押しでスタックトップを消します (DROP)
- '/' の長押しでスタックトップの２つを入れ替えます (SWAP)
- '*' の長押しで小数・分数表示モードを切り替えます（再掲）
