#include <stdio.h>
#include <ruby.h>
#include <sapi/embed/php_embed.h>


static int php_ub_write(const char *str, unsigned int str_length TSRMLS_DC)
{
  printf("php_output: %s\n", str);
  return str_length;
}
static void php_log_message(char *message)
{
  printf("php_log: %s\n", message);
}
static void php_sapi_error(int type, const char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    printf("php_sapi_error: ");
    vprintf(fmt, va);
    va_end(va);
}

VALUE zval_to_value(zval* val) {
  VALUE r;

  switch(Z_TYPE_P(val)) {
    case IS_NULL:
      return Qnil;
    case IS_BOOL:
      return (zval_is_true(val)) ? Qtrue : Qfalse;
    case IS_LONG:
      return INT2NUM(Z_LVAL_P(val));
    case IS_DOUBLE:
      return DBL2NUM(Z_DVAL_P(val));
    case IS_ARRAY:
      {
        HashTable* ht = Z_ARRVAL_P(val);
        HashPosition p;
        int idx = 0;
        zval** data;

        r = rb_ary_new2(zend_hash_num_elements(ht));

        zend_hash_internal_pointer_reset_ex(ht, &p);
        while (zend_hash_get_current_data_ex(ht, (void **)&data, &p) == SUCCESS) {
          VALUE t = zval_to_value(*data);
          rb_ary_store(r, idx++, t);
          zend_hash_move_forward_ex(ht, &p);
        }
        return r; 
      }
    case IS_OBJECT:
    case IS_STRING:
    case IS_RESOURCE:
    case IS_CONSTANT:
    case IS_CONSTANT_ARRAY:
    default:
      convert_to_string(val);
      return rb_str_new(Z_STRVAL_P(val), Z_STRLEN_P(val));
  }
}

void value2php_arg(VALUE v, char* out_arg) {
  switch (TYPE(v)) {
    case T_FALSE:
    	strcat(out_arg, "false");
      return;
    case T_TRUE:
    	strcat(out_arg, "true");
      return;
    case T_UNDEF:
    case T_NIL:
    	strcat(out_arg, "null");
      return;
    case T_FIXNUM:
      {
        VALUE t = rb_fix2str(v, 10);
        strcat(out_arg, StringValuePtr(t));
      }
      return;
    case T_BIGNUM:
    case T_FLOAT:
      {
        VALUE t = rb_big2str(v, 10);
        strcat(out_arg, StringValuePtr(t));
      }
      return;
    case T_ARRAY:
      {
        int i;
        strcat(out_arg, "array(");
        for(i=0;i<RARRAY_LEN(v);++i) {
          value2php_arg(RARRAY_PTR(v)[i], out_arg);
          if (i != RARRAY_LEN(v)-1) {
            strcat(out_arg, ",");
          }
        }
        strcat(out_arg, ")");
      }
    case T_HASH:
      /* no implement */
      return;
    case T_STRING:
    default:
      strcat(out_arg, "'");
      strcat(out_arg, StringValuePtr(v));
      strcat(out_arg, "'");
  }
}

int eval_php_code(char* code, const char* fetch_variable_name, VALUE* fetch_value) {
  const char *argv[2] = {"ruby", NULL};
  int err = 0;
  zval **data;

  PHP_EMBED_START_BLOCK(1, (char**)argv);

  if (zend_eval_string(code, NULL, (char*)"" TSRMLS_CC) == FAILURE) {
    err = 1;
  }

  if (NULL != fetch_variable_name) {
    if (zend_hash_find(&EG(symbol_table), fetch_variable_name, strlen(fetch_variable_name) + 1, (void **)&data) != FAILURE) {
      *fetch_value = zval_to_value(*data);
    }
  }

  PHP_EMBED_END_BLOCK();

  return err;
}

VALUE php_eval(VALUE self, VALUE code) {
  const char *argv[2] = {"ruby", NULL};
  
  if (eval_php_code(StringValuePtr(code), NULL, NULL)) {
    rb_raise(rb_eRuntimeError, "invalid code");
  }

  return Qtrue;
}

VALUE php_call(int argc, VALUE *argv, VALUE self) {
  VALUE func, args, retval;
  int i;
  char arg_str[1024], call_str[2048];
  zval* t = NULL;

  rb_scan_args(argc, argv, "1*", &func, &args);

  
  arg_str[0] = '\0';
  for(i=0; i<argc-1; ++i) {
    char arg[255] = "";
    value2php_arg(RARRAY_PTR(args)[i], arg);
    strcat(arg_str, arg);
    
    if (i != argc-2) {
      strcat(arg_str, ",");
    }
  }

  sprintf(call_str, "$retval = %s(%s);", StringValuePtr(func), arg_str);
  printf("\n***%s***\n", call_str);
  if (eval_php_code(call_str, "retval", &retval)) {
    rb_raise(rb_eRuntimeError, "eval error");
  }

  return retval;
}



Init_Php() {

  VALUE cPhp;
  cPhp = rb_define_class("Php", rb_cObject);
  rb_define_const(cPhp, "VERSION", rb_ary_new3(3, INT2FIX(0), INT2FIX(0), INT2FIX(1)));

  rb_define_method(cPhp, "eval", php_eval, 1);
  rb_define_method(cPhp, "call", php_call, -1);

  


  php_embed_module.ub_write = php_ub_write;
  php_embed_module.log_message = php_log_message;
  php_embed_module.sapi_error = php_sapi_error;

}



