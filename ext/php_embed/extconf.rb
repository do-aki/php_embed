require 'mkmf'
require 'fileutils'
require 'pathname'
require 'open-uri'



PHP_VERSION = '5.4.10'
PHP_SRC_URL = "http://php.net/distributions/php-#{PHP_VERSION}.tar.bz2"

def download_src(destfile)
  print "download php source\n"
  FileUtils.copy_stream(open(PHP_SRC_URL), destfile)
end

def decompression(archive)
  print "decompression php source\n"
  `tar xf #{archive}`
  abort 'tar failure' if $?.exitstatus != 0
end

def configure(prefix)
  print "configure\n"

  opts = %w(
    --disable-cgi
    --without-pear
    --enable-sockets
    --enable-ftp
    --with-mysql=mysqlnd
    --with-mysqli=mysqlnd
    --with-pdo-mysql=mysqlnd
    --enable-pcntl
    --enable-mbstring
    --disable-debug
    --disable-libxml
    --disable-dom
    --disable-simplexml
    --disable-xml
    --disable-xmlreader
    --disable-xmlwriter
    --disable-phar
    --enable-embed
  ).join(' ')
  opts += " --prefix=#{prefix}"
  opts += " --with-libdir=lib64"  if `uname -p`.chomp == 'x86_64'

  `./configure #{opts}`
  abort 'configure failure' if $?.exitstatus != 0
end

def make_and_install
  print "make\n"
  `make`
  abort 'make failure' if $?.exitstatus != 0

  print "make install\n"
  `make install`
  abort 'make install failure' if $?.exitstatus != 0
end

def prepare_compile_php(prefix)
  abort 'need tar' unless find_executable 'tar'
  abort 'need make' unless find_executable 'make'
end

def compile_php(prefix)
  Dir.mkdir 'src' unless Dir.exists? 'src'
  Dir.chdir('src') do
    src_filename = "php-#{PHP_VERSION}.tar.bz2"
    download_src(src_filename) unless File.exists? src_filename
    decompression(src_filename)
  end

  src_dir = "src/php-#{PHP_VERSION}"
  if !Dir.exists? src_dir
    abort 'soruce directory not exists'
  end

  FileUtils.mkpath "#{prefix}/etc/conf.d"

  Dir.chdir(src_dir) do
    configure(prefix)
    make_and_install
  end

  FileUtils.copy 'php.ini', "#{prefix}/lib/"
end




dir_config('php')
php_config = find_executable('php-config')
have_libphp = have_library('php5', 'php_embed_init')

if !php_config || !have_libphp
  php_version = arg_config('--compile-php')
  abort 'libphp5 or php-config not found: try --compile-php' unless php_version

  prefix = Pathname.getwd.join('php')

  prepare_compile_php(prefix.to_s)
  compile_php(prefix.to_s)

  php_config = prefix.join('bin', 'php-config').to_s
  find_library('php5', 'php_embed_init', prefix.join('lib').to_s)
end

$CPPFLAGS = `#{php_config} --includes`.chomp + ' ' + $CPPFLAGS

create_makefile("php_embed/php")

