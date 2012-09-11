#include <stdio.h>
#include <ruby.h>
#include <sapi/embed/php_embed.h>

#include <php_embed_value.h>
VALUE zval_to_value(zval* val);

static VALUE callback_output = Qnil;
static VALUE callback_error = Qnil;

static int php_ub_write(const char *str, unsigned int str_length TSRMLS_DC)
{
  if (!NIL_P(callback_output)) {
    VALUE args = rb_ary_new();
    rb_ary_push(args, rb_str_new(str, str_length));
    rb_proc_call(callback_output, args);
  }
  return str_length;
}
static void php_log_message(char *message)
{
  if (!NIL_P(callback_error)) {
    rb_proc_call(callback_error, rb_ary_new3(1, rb_str_new(message, strlen(message))));
  }
}
static void php_sapi_error(int type, const char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    printf("php_sapi_error: ");
    vprintf(fmt, va);
    va_end(va);
}

static int is_array_convertable(HashTable* ht) {
  HashPosition pos;
  char  *string_key;
  ulong num_index;
  ulong index = 0;

  zend_hash_internal_pointer_reset_ex(ht, &pos);
  do {
    switch(zend_hash_get_current_key_ex(ht, &string_key, NULL, &num_index, 0, &pos)) {
      case HASH_KEY_IS_STRING:
        return 0;
      case HASH_KEY_NON_EXISTANT:
        return 1;
      case HASH_KEY_IS_LONG:
        if (num_index != index) {
          return 0;
        }
        ++index;
    }
  } while(SUCCESS == zend_hash_move_forward_ex(ht, &pos));
  return 1;
}

static VALUE hash_to_array(HashTable* ht) {
  HashPosition pos;
  zval** data;
  VALUE ret;

  ret = rb_ary_new2(zend_hash_num_elements(ht));

  zend_hash_internal_pointer_reset_ex(ht, &pos);
  while (SUCCESS == zend_hash_get_current_data_ex(ht, (void **)&data, &pos)) {
    VALUE t = zval_to_value(*data);
    rb_ary_push(ret, t);
    zend_hash_move_forward_ex(ht, &pos);
  }
  return ret;
}

static VALUE hash_to_hash(HashTable* ht) {
  HashPosition pos;
  zval** data;
  VALUE ret;

  ret = rb_hash_new();

  zend_hash_internal_pointer_reset_ex(ht, &pos);
  while (SUCCESS == zend_hash_get_current_data_ex(ht, (void **)&data, &pos)) {
    char* string_key;
    ulong num_index;
    VALUE key = Qnil;
    VALUE val = zval_to_value(*data);

    switch(zend_hash_get_current_key_ex(ht, &string_key, NULL, &num_index, 0, &pos)) {
      case HASH_KEY_IS_STRING:
        key = rb_str_new_cstr(string_key);
        break;
      case HASH_KEY_IS_LONG:
        key = LONG2NUM(num_index);
        break;
    }

    rb_hash_aset(ret, key, val);
    zend_hash_move_forward_ex(ht, &pos);
  }
  return ret;
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
    case IS_CONSTANT_ARRAY:
      {
        HashTable* ht = Z_ARRVAL_P(val);

        if (0 == zend_hash_num_elements(ht)) {
          return rb_ary_new();
        }

        if (is_array_convertable(ht)) {
          return hash_to_array(ht);
        }

        return hash_to_hash(ht);
      }
    case IS_OBJECT:
    case IS_RESOURCE:
    case IS_CONSTANT:
      convert_to_string(val);
    case IS_STRING:
      return rb_str_new(Z_STRVAL_P(val), Z_STRLEN_P(val));
    default:
      return Qnil;
  }
}

int eval_php_code(char* code) {
  int ret = 0;

  zend_try {
    ret = zend_eval_string(code, NULL, (char*)"" TSRMLS_CC);
  } zend_catch {

  } zend_end_try();

  return FAILURE == ret;
}

int eval_and_return_php_code(char* code, VALUE* return_value) {
  int err = 0;
  zval retval;

  zend_try {
    if (zend_eval_string(code, &retval, (char*)"" TSRMLS_CC) == FAILURE) {
      err = 1;
    } else {
      *return_value = zval_to_value(&retval);
      zval_dtor(&retval);
    }

  } zend_catch {

  } zend_end_try();

  return err;
}

VALUE php_eval(VALUE self, VALUE code) {
  if (eval_php_code(StringValuePtr(code))) {
    rb_raise(rb_eRuntimeError, "invalid code");
  }

  return Qnil;
}

VALUE php_call(int argc, VALUE *argv, VALUE self) {
  VALUE func, args, arg_str, retval;
  int i;
  char* call_str;

  rb_scan_args(argc, argv, "1*", &func, &args);

  if (T_SYMBOL == TYPE(func)) {
    func = rb_sym_to_s(func); 
  }

  if (T_STRING != TYPE(func)) {
    rb_raise(rb_eRuntimeError, "invalid function name");
  }

  arg_str = rb_str_new_cstr("");
  for(i=0; i<argc-1; ++i) {
    VALUE r = convert_value_into_php_string(RARRAY_PTR(args)[i]);
    rb_str_cat(arg_str, RSTRING_PTR(r), RSTRING_LEN(r));
    
    if (i != argc-2) {
      rb_str_cat2(arg_str, ",");
    }
  }

  call_str = malloc(RSTRING_LEN(func) + RSTRING_LEN(arg_str) + sizeof("()") + 1);
  sprintf(call_str, "%s(%s)", RSTRING_PTR(func), RSTRING_PTR(arg_str));
  if (eval_and_return_php_code(call_str, &retval)) {
    rb_raise(rb_eRuntimeError, "eval error");
  }

  return retval;
}

VALUE php_run(VALUE self, VALUE file) {
  VALUE retval = Qtrue;
  zend_file_handle handle;

  if (T_STRING != TYPE(file)) {
    rb_raise(rb_eRuntimeError, "file must be string");
  }

  handle.type = ZEND_HANDLE_FILENAME;
  handle.filename = RSTRING_PTR(file);
  handle.opened_path = NULL;
  handle.free_filename = 0;
  
  zend_try {
    zend_execute_scripts(ZEND_REQUIRE TSRMLS_CC, NULL, 1, &handle);
  } zend_catch {
    retval = Qfalse;
  } zend_end_try();

  return retval;

}

VALUE php_fetch_variable(VALUE self, VALUE name) {
  zval *data = NULL;
  if (FAILURE == zend_hash_find(&EG(symbol_table), StringValuePtr(name), RSTRING_LEN(name), (void **)&data)) {
    /* Name not found in $GLOBALS */
  }
  if (data == NULL) {
    /* Value is NULL (not possible for symbol_table?) */
  }
 
  return Qnil;
}

VALUE php_set_output_handler(VALUE self, VALUE callback) {
  if (rb_obj_is_proc(callback)) {
    callback_output = callback;
  } else {
    rb_raise(rb_eArgError, "callback must be proc");
  }
  
  return Qnil;
}

VALUE php_set_error_handler(VALUE self, VALUE callback) {
  if (rb_obj_is_proc(callback)) {
    callback_error = callback;
  } else {
    rb_raise(rb_eArgError, "callback must be proc");
  }
  
  return Qnil;
}

void initialize_php_embed() {
  const char *argv[2] = {"ruby", NULL};
  php_embed_init(1, (char**)argv);
  EG(bailout)=NULL;
}

void shutdown_php_embed() {
  php_embed_shutdown(TSRMLS_C);
}

VALUE mPhpEmbed;

Init_php() {

  mPhpEmbed = rb_define_module("PhpEmbed");

  init_php_value();

  //rb_define_const(mPhpEmbed, "VERSION", rb_ary_new3(3, INT2FIX(0), INT2FIX(0), INT2FIX(1)));

  rb_define_singleton_method(mPhpEmbed, "eval", php_eval, 1);
  rb_define_singleton_method(mPhpEmbed, "call", php_call, -1);
  rb_define_singleton_method(mPhpEmbed, "run", php_run, 1);
  rb_define_singleton_method(mPhpEmbed, "fetchVariable", php_fetch_variable, 1);
  rb_define_singleton_method(mPhpEmbed, "setOutputHandler", php_set_output_handler, 1);
  rb_define_singleton_method(mPhpEmbed, "setErrorHandler", php_set_error_handler, 1);
   
  php_embed_module.ub_write = php_ub_write;
  php_embed_module.log_message = php_log_message;
  php_embed_module.sapi_error = php_sapi_error;

  initialize_php_embed();
  atexit(shutdown_php_embed);

/*
  zend_try {
    zend_alter_ini_entry((char*)"display_errors", sizeof("display_errors")
      , (char*)"0", sizeof("0")-1, PHP_INI_SYSTEM, PHP_INI_STAGE_RUNTIME);
    zend_alter_ini_entry((char*)"log_errors", sizeof("log_errors")
      , (char*)"1", sizeof("1")-1, PHP_INI_SYSTEM, PHP_INI_STAGE_RUNTIME);
  } zend_catch {
  } zend_end_try();
*/
}



