require "mkmf"


dir_config('php')
if !have_library('php5', 'php_embed_init')
    exit(1)
end

$CPPFLAGS = `php-config --includes`.chomp + ' ' + $CPPFLAGS

create_makefile("Php")
