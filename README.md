php_embed
============

Ruby 上で PHP のコードを実行します

Synopsis
-----------
```ruby
require 'php_embed'

puts PhpEmbed.call("phpversion") # output return value of phpversion function


PhpEmbed.eval('function hello_php(){ return "php world!";}')
puts PhpEmbed.call("hello_php") # "php world!"


PhpEmbed.setOutputHandler(Proc.new { |output|
    puts output
})
PhpEmbed.call("phpinfo") # output phpinfo()

```

Requirements
-----------
* ruby >= 1.9
* php embed-sapi (optional)


About php embed SAPI
----------
php をソースからコンパイルする場合、そのままでは embed-sapi はコンパイルされません。
configure 時に、 --enable-embed オプションを指定する必要があります。

CentOS6 であれば `php-embedded` と `php-devel` パッケージでも代替可能です。


Installation
-----------

###normal install
```
gem install php_embed
```
システムの php embed-sapi 共有ライブラリを利用します
利用出来ない場合、インストールは失敗します


###install with php compile
```
gem install php_embed -- --compile-php
```

インストール時に、php のソースコードを取得してコンパイルします
インストールされる php のバージョンのデフォルトは 5.6.0 です。

`--compile-php=5.5.17` のように、php のバージョンを指定することも可能です


##install with php compile (use old version)
```
gem install php_embed -- --compile-php=5.3.3 --php-source-url=http://museum.php.net/php5/php-5.3.3.tar.bz2
```
古いバージョンの php を使いたい場合、ソースコードがダウンロードできずにインストールに失敗する場合があります
その時は `--php-source-url` でダウンロードURLを指定できます

Usage
-----------
spec ディレクトリにある rspec を参考にしてください。


### Slide 
http://www.slideshare.net/do_aki/php-in-ruby

Slide 中の php-embed となっているところは php_embed の誤りです。


