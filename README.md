ARES - JR線運賃計算アプリケーション
===================================
概要
------------
ARESはJRの運賃計算をするためのアプリケーションです。
普通に考えると運賃計算など現在ではどのWebポータルも実装していてもはや喫緊のプログラムではないような気がすると思います。しかし、JRの運賃計算には数多くの例外事項を含んでいるために、これらをすべて検討した上でいかなる時にも正確な運賃を計算できるようなソフトウェアはそう多くはありません。

現在現実に存在している運賃計算アプリケーションの中で最も正確にJRの運賃を計算できるのはおそらく[MARS for DOS](http://www.swa.gr.jp/pub/mars/ "SWA - MARS for DOS")だと思われます。しかしMARSはMS-DOS用ソフトウェアであり、普及しつつある64bit版Windowsでは動作しません。また、近年ではWindows以外のOSのシェアも向上してきており、作者もLinuxを通常では利用しているため、MARSを利用するためにわざわざ仮想マシンを利用する始末です。このような状況を受けてよりマルチプラットフォームで正確な運賃計算アプリケーションの必要性が高まっているのですが、MARSはプロプライエタリなソフトウェアなので作者以外にはこれらの状況への対応が事実上不可能になっています。

今回実装するARESはこれらを解決するためにマルチプラットフォームな外部ライブラリのみに依存し、広い環境で動作する事を目指した運賃計算ソフトウェアです。作者は開発にGNU/Linuxを用いていますが、運賃計算部分とUI部分を明確に分離することで簡単に他環境へ移植出来ることを目指しています。

使用方法
------------
未実装です。
### インストール ###
### 実行 ###

実装
------------
### 概要 ###
鉄道路線に関する情報は作者はCSV形式で保存し、これをPythonで実装したスクリプトによって事前にSQLite3のデータベースに変換する。実際の実行の際にはこのデータベースを利用する。
