#ifndef PHP_EMBED_VALUE
#define PHP_EMBED_VALUE

extern VALUE cPhpEmbedValue;

void init_php_value();
VALUE new_php_embed_value(zval *value);

typedef struct {
    zval *value;
} php_value;

#endif

