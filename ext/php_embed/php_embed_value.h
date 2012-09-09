#ifndef PHP_EMBED_VALUE
#define PHP_EMBED_VALUE

void init_php_value();

VALUE convert_value_into_php_string(VALUE v);

typedef struct {
  VALUE value;
} php_value;

#endif

