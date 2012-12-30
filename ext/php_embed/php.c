#include <stdio.h>
#include <ruby.h>
#include <sapi/embed/php_embed.h>
#include "value.h"
#include "convert.h"

static VALUE callback_output = Qnil;
static VALUE callback_error = Qnil;


VALUE mPhpEmbed;

VALUE rb_ePhpEmbedStanderdError;
VALUE rb_ePhpEmbedSyntaxError;
VALUE rb_ePhpEmbedMissingError;

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
      //*return_value = zval_to_value(&retval);
      *return_value = new_php_embed_value(&retval);
      zval_dtor(&retval);
    }

  } zend_catch {

  } zend_end_try();

  return err;
}

VALUE php_eval(VALUE self, VALUE code) {
  if (eval_php_code(StringValuePtr(code))) {
    rb_raise(rb_ePhpEmbedSyntaxError, "invalid code");
  }

  return Qnil;
}


zend_function* php_find_function(char* name) {
  char* lcname;
  int name_len;
  int found;
  zend_function *func;

  name_len = strlen(name);
  lcname = zend_str_tolower_dup(name, name_len);

  name = lcname;
  if ('\\' == lcname[0]) {
    name = &lcname[1];
    name_len--;
  }

  found = (SUCCESS == zend_hash_find(EG(function_table), name, name_len+1, (void**)&func));
  efree(lcname);

  if (found) {
    return func;
  }

  return NULL;
}



VALUE php_call(int argc, VALUE *argv, VALUE self) {
  VALUE name, args, retval;
  zend_function* func;
  int call_result, i;
  zval *retval_ptr;
  zend_fcall_info fci;
  zend_fcall_info_cache fcc;
  zval ***call_args;
  zval **zval_array;

  rb_scan_args(argc, argv, "1*", &name, &args);

  if (T_SYMBOL == TYPE(name)) {
    name = rb_sym_to_s(name); 
  }

  if (T_STRING != TYPE(name)) {
    rb_raise(rb_eArgError, "invalid function name");
  }


  func = php_find_function(StringValueCStr(name));
  if (!func) {
    rb_raise(rb_ePhpEmbedMissingError, "function not found");
  }

  zval_array = (zval**)malloc(sizeof(zval*) * argc-1); 
  call_args = (zval***)malloc(sizeof(zval**) * argc-1); 
  for(i=0; i<argc-1; ++i) {
    zval_array[i] = value_to_zval(RARRAY_PTR(args)[i]);
    call_args[i] = &zval_array[i];
  }
  
  fci.size = sizeof(fci);
  fci.function_table = NULL;
  fci.function_name = NULL;
  fci.symbol_table = NULL;
  fci.object_ptr = NULL;
  fci.retval_ptr_ptr = &retval_ptr;
  fci.param_count = argc-1;
  fci.params = call_args;
  fci.no_separation = 1;

  fcc.initialized = 1;
  fcc.function_handler = func;
  fcc.calling_scope = EG(scope);
  fcc.called_scope = NULL;
  fcc.object_ptr = NULL;

  call_result = zend_call_function(&fci, &fcc TSRMLS_CC);
  retval = new_php_embed_value(retval_ptr);

  free(call_args);

  for(i=0; i<argc-1; ++i) {
    zval_dtor(zval_array[i]);
    FREE_ZVAL(zval_array[i]);
  }
  free(zval_array);

  if (FAILURE == call_result) {
    rb_raise(rb_ePhpEmbedStanderdError, "function call fairure");
  }

  return retval;
}

VALUE php_require(VALUE self, VALUE file) {
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


Init_php() {

  mPhpEmbed = rb_define_module("PhpEmbed");

  init_php_value();

  //rb_define_const(mPhpEmbed, "VERSION", rb_ary_new3(3, INT2FIX(0), INT2FIX(0), INT2FIX(1)));

  rb_define_singleton_method(mPhpEmbed, "eval", php_eval, 1);
  rb_define_singleton_method(mPhpEmbed, "call", php_call, -1);
  rb_define_singleton_method(mPhpEmbed, "require", php_require, 1);
  rb_define_singleton_method(mPhpEmbed, "fetchVariable", php_fetch_variable, 1);
  rb_define_singleton_method(mPhpEmbed, "setOutputHandler", php_set_output_handler, 1);
  rb_define_singleton_method(mPhpEmbed, "setErrorHandler", php_set_error_handler, 1);
   
  php_embed_module.ub_write = php_ub_write;
  php_embed_module.log_message = php_log_message;
  php_embed_module.sapi_error = php_sapi_error;

  initialize_php_embed();
  atexit(shutdown_php_embed);


  rb_ePhpEmbedStanderdError = rb_define_class_under(mPhpEmbed, "StanderdError", rb_eException);
  rb_ePhpEmbedSyntaxError = rb_define_class_under(mPhpEmbed, "SyntaxError", rb_ePhpEmbedStanderdError);
  rb_ePhpEmbedMissingError = rb_define_class_under(mPhpEmbed, "MissingError", rb_ePhpEmbedStanderdError);
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



