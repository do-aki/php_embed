#ifndef PHP_EMBED_HEADER
#define PHP_EMBED_HEADER

#include <ruby.h>
#include <php.h>
#include "value.h"
#include "convert.h"

extern VALUE mPhpEmbed;
extern VALUE rb_ePhpEmbedStanderdError;
extern VALUE rb_ePhpEmbedSyntaxError;
extern VALUE rb_ePhpEmbedMissingError;

int eval_php_code(char* code);
int eval_and_return_php_code(char* code, VALUE* return_value);

#endif


