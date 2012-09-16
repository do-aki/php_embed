php_embed
============

Ruby 上で PHP のコードを実行します

Requirements
-----------
* ruby >= 1.9
* php embed-sapi

ruby 1.9 系でしか動きません。 

コンパイル時に phpize が、
実行に php embed-sapi 共有ライブラリが、
それぞれ必要となります。

php をソースからコンパイルする場合、そのままでは embed-sapi はコンパイルされません。
configure 時に、 --enable-embed オプションを指定してください。

CentOS6 であれば php-embeded と php-devel パッケージでも代替可能です。


Installation
-----------
1. prepare php embed-sapi
2. **gem install php_embed**

Usage
-----------
spec ディレクトリにある rspec を参考にしてください。


### Slide 
http://www.slideshare.net/do_aki/php-in-ruby

php-embed となっているところは php_embed の誤りです。

