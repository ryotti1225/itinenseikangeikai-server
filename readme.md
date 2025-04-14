＜==初期設定==＞

①git で vcpkg をインストール
git clone https://github.com/Microsoft/vcpkg.git
②vcpkg のディレクトリに移動
cd vcpkg
③bootstrap-vcpkg.bat を実行
bootstrap-vcpkg.bat

=====================================================================

＜==ライブラリ追加時==＞

①cmd を立ち上げて vcpkg のディレクトリに移動する
cd vcpkg
②vcpkg コマンドを使用してほしいパッケージをインストール
vcpkg install パッケージ名
③visual studio と同期する
vcpkg integrate install

(これでパッケージが visual studio で動く)

=====================================================================
