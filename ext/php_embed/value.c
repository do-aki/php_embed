#include <string.h>
#include "php_embed.h"

VALUE cPhpEmbedValue;

static void php_value_mark(php_value* pv) {
  if (pv) {
    pv->value = NULL;
    //rb_gc_mark(pv->value);
  }
}

static void php_value_free(php_value* pv) {
  if (pv) {
    if (NULL != pv->value) {
      zval_dtor(pv->value);
      FREE_ZVAL(pv->value);
    }
    xfree(pv);
  }
}

static VALUE php_value_alloc(VALUE klass) {
  php_value * pv;
  return Data_Make_Struct(klass, php_value, php_value_mark, php_value_free, pv);
}

static zval* get_zval(VALUE value) {
  php_value* pv;
  Data_Get_Struct(value, php_value, pv);
  return pv->value;
}

static void set_zval(VALUE value, zval* zv) {
  php_value* pv;
  Data_Get_Struct(value, php_value, pv);
  pv->value = zv;
}


VALUE new_php_embed_value(zval* value) {

  VALUE new_value;
  zval* new_zval;
  VALUE arg = INT2FIX(0);

  ALLOC_ZVAL(new_zval);
  *new_zval = *value;
  zval_copy_ctor(new_zval);
  INIT_PZVAL(new_zval);
  
  new_value = php_value_alloc(cPhpEmbedValue);
  set_zval(new_value, new_zval);

  return new_value;
}

VALUE php_value_initialize(VALUE self, VALUE value) {
  zval* retval;
  VALUE code;

  set_zval(self, value_to_zval(value));

  return Qnil;
}

VALUE php_value_callable(VALUE self) {
  char *error;
  zend_bool retval;

  retval = zend_is_callable_ex(get_zval(self), NULL, 0, NULL, NULL, NULL, &error TSRMLS_CC);
  if (error) {
    efree(error);  
  }

  return (retval) ? Qtrue : Qfalse;
}

VALUE php_value_call(int argc, VALUE *argv, VALUE self) {
  char *is_callable_error;
  zend_fcall_info fci;
  zend_fcall_info_cache fcc;
  zval ***call_args;
  zval **zval_array;
  zval *retval_ptr = NULL;
  int i, call_result;
  VALUE args, retval;

  rb_scan_args(argc, argv, "*", &args);

  if (FAILURE == zend_fcall_info_init(get_zval(self), 0, &fci, &fcc, NULL, &is_callable_error TSRMLS_CC)) {
    VALUE err = rb_str_new2(is_callable_error);
    efree(is_callable_error);
    rb_raise(rb_eRuntimeError, "no callable: %s", err);
  }
  
  zval_array = (zval**)malloc(sizeof(zval*) * argc);
  call_args = (zval***)malloc(sizeof(zval**) * argc);
  for(i=0; i<argc; ++i) {
    zval_array[i] = value_to_zval(RARRAY_PTR(args)[i]);
    call_args[i] = &zval_array[i];
  }

  fci.retval_ptr_ptr = &retval_ptr;
  fci.param_count = argc;
  fci.params = call_args;

  call_result = zend_call_function(&fci, &fcc TSRMLS_CC);
  retval = new_php_embed_value(retval_ptr);

  free(call_args);

  for(i=0; i<argc-1; ++i) {
    zval_dtor(zval_array[i]);
    FREE_ZVAL(zval_array[i]);
  }
  free(zval_array);

  if (FAILURE == call_result) {
    rb_raise(rb_eRuntimeError, "function call fairure");
  }

  return retval;
}


VALUE php_value_to_php(VALUE self, VALUE value) {
  return convert_value_to_php_string(value);
}

VALUE php_value_to_string(VALUE self) {
  zval* zv = get_zval(self);

  if (zv) {
    convert_to_string(zv);
    return rb_str_new(Z_STRVAL_P(zv), Z_STRLEN_P(zv)); 
  }

  return Qnil;
}

VALUE php_value_eval(VALUE self, VALUE src) {
  VALUE return_value;
  char* code = StringValueCStr(src);

  if (eval_and_return_php_code(code, &return_value) == FAILURE) {
    rb_raise(rb_ePhpEmbedStanderdError, "eval failed");
  }

  return return_value;
}

VALUE php_value_to_integer(VALUE self) {
  zval* zv = get_zval(self);
    
  if (zv) {
    convert_to_long(zv);
    return rb_fix_new(Z_LVAL_P(zv)); 
  }

  return Qnil;
}

VALUE php_value_to_float(VALUE self) {
  zval* zv = get_zval(self);
    
  if (zv) {
    convert_to_double(zv);
    return rb_float_new(Z_DVAL_P(zv)); 
  }

  return Qnil;
}

VALUE php_value_to_boolean(VALUE self) {
  zval* zv = get_zval(self);
    
  if (zv) {
    convert_to_boolean(zv);
    if (Z_LVAL_P(zv)) {
      return Qtrue;
    } else {
      return Qfalse;
    }
  }

  return Qnil;
}

VALUE php_value_to_array(VALUE self) {
  zval* zv = get_zval(self);
 
  if (zv) {
    return zval_to_array(zv);
  }

  return Qnil;
}

VALUE php_value_to_hash(VALUE self) {
  zval* zv = get_zval(self);
 
  if (zv) {
    return zval_to_hash(zv);
  }

  return Qnil;
}

VALUE php_value_obj_equal(VALUE self, VALUE rhs) {
  zval* lhs_zv;
  zval* rhs_zv;
  zval* result;    
  int cmp_ret;

  lhs_zv = get_zval(self);
  if (lhs_zv == NULL) {
    return Qnil;
  }

  if (CLASS_OF(rhs) == cPhpEmbedValue) {
    rhs_zv = get_zval(rhs);
  } else {
    rhs_zv = value_to_zval(rhs);
  }

  MAKE_STD_ZVAL(result);
  compare_function(result, lhs_zv, rhs_zv);
  cmp_ret = Z_LVAL_P(result);
  FREE_ZVAL(result); 

  return cmp_ret == 0 ? Qtrue: Qfalse;
}

void init_php_value() {
#if 0
  mPhpEmbed = rb_define_module("PhpEmbed");
#endif

  cPhpEmbedValue = rb_define_class_under(mPhpEmbed, "Value", rb_cObject);
  rb_define_alloc_func(cPhpEmbedValue, php_value_alloc);
  rb_define_method(cPhpEmbedValue, "initialize", php_value_initialize, 1);
  rb_define_method(cPhpEmbedValue, "callable?", php_value_callable, 0);
  rb_define_method(cPhpEmbedValue, "call", php_value_call, -1);
  rb_define_method(cPhpEmbedValue, "to_s", php_value_to_string, 0);
  rb_define_method(cPhpEmbedValue, "to_i", php_value_to_integer, 0);
  rb_define_method(cPhpEmbedValue, "to_f", php_value_to_float, 0);
  rb_define_method(cPhpEmbedValue, "to_b", php_value_to_boolean, 0);
  rb_define_method(cPhpEmbedValue, "to_a", php_value_to_array, 0);
  rb_define_method(cPhpEmbedValue, "to_h", php_value_to_hash, 0);
  rb_define_method(cPhpEmbedValue, "==", php_value_obj_equal, 1);

  rb_define_singleton_method(cPhpEmbedValue, "to_php", php_value_to_php, 1);
  rb_define_singleton_method(cPhpEmbedValue, "eval", php_value_eval, 1);
}
