#ifndef PHP_EMBED_CONVERT
#define PHP_EMBED_CONVERT

#include <ruby.h>
#include <php.h>


VALUE zval_to_hash(zval* zv);
VALUE zval_to_array(zval* zv);

VALUE convert_value_to_php_string(VALUE v);
zval* value_to_zval(VALUE v);

#endif
