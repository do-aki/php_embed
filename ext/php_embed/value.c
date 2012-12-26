#include <string.h>
#include <ruby.h>
#include <php.h>
#include "value.h"
#include "convert.h"

extern mPhpEmbed;
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


VALUE new_php_embed_value(zval* value) {

  VALUE new_value;
  php_value* pv;
  zval* new_zval;
  VALUE arg = INT2FIX(0);

  ALLOC_ZVAL(new_zval);
  *new_zval = *value;
  zval_copy_ctor(new_zval);
  INIT_PZVAL(new_zval);
  
  new_value = php_value_alloc(cPhpEmbedValue);
  Data_Get_Struct(new_value, php_value, pv);
  pv->value = new_zval;

  return new_value;
}


VALUE php_value_initialize(VALUE self, VALUE value) {
  php_value* pv;
  zval* retval;
  VALUE code;

  Data_Get_Struct(self, php_value, pv);
  pv->value = value_to_zval(value);

  return Qnil;
}

VALUE php_value_to_php(VALUE self, VALUE value) {
  return convert_value_to_php_string(value);
}

VALUE php_value_to_string(VALUE self) {
  php_value* pv;
  Data_Get_Struct(self, php_value, pv);
 
    
  if (pv->value) {
    convert_to_string(pv->value);
    return rb_str_new(Z_STRVAL_P(pv->value), Z_STRLEN_P(pv->value)); 
  }

  return Qnil;
}

VALUE php_value_to_integer(VALUE self) {
  php_value* pv;
  Data_Get_Struct(self, php_value, pv);
 
    
  if (pv->value) {
    convert_to_long(pv->value);
    return rb_fix_new(Z_LVAL_P(pv->value)); 
  }

  return Qnil;
}

VALUE php_value_to_float(VALUE self) {
  php_value* pv;
  Data_Get_Struct(self, php_value, pv);
 
    
  if (pv->value) {
    convert_to_double(pv->value);
    return rb_float_new(Z_DVAL_P(pv->value)); 
  }

  return Qnil;
}

VALUE php_value_to_boolean(VALUE self) {
  php_value* pv;
  Data_Get_Struct(self, php_value, pv);
    
  if (pv->value) {
    convert_to_boolean(pv->value);
    if (Z_LVAL_P(pv->value)) {
      return Qtrue;
    } else {
      return Qfalse;
    }
  }

  return Qnil;
}

VALUE php_value_to_array(VALUE self) {
  php_value* pv;
  Data_Get_Struct(self, php_value, pv);
 
  if (pv->value) {
    return zval_to_array(pv->value);
  }

  return Qnil;
}

VALUE php_value_to_hash(VALUE self) {
  php_value* pv;
  Data_Get_Struct(self, php_value, pv);
 
  if (pv->value) {
    return zval_to_hash(pv->value);
  }

  return Qnil;
}

VALUE php_value_obj_equal(VALUE self, VALUE rhs) {
  php_value* pv;
  zval* rhs_zv;
  zval* result;    
  int cmp_ret;

  Data_Get_Struct(self, php_value, pv);
  if (pv->value == NULL) {
    return Qnil;
  }

  if (CLASS_OF(rhs) == cPhpEmbedValue) {
    php_value* rhs_pv;
    Data_Get_Struct(rhs, php_value, rhs_pv);

    rhs_zv = rhs_pv->value;
  } else {
    rhs_zv = value_to_zval(rhs);
  }

  MAKE_STD_ZVAL(result);
  compare_function(result, pv->value, rhs_zv);
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
  rb_define_method(cPhpEmbedValue, "to_s", php_value_to_string, 0);
  rb_define_method(cPhpEmbedValue, "to_i", php_value_to_integer, 0);
  rb_define_method(cPhpEmbedValue, "to_f", php_value_to_float, 0);
  rb_define_method(cPhpEmbedValue, "to_b", php_value_to_boolean, 0);
  rb_define_method(cPhpEmbedValue, "to_a", php_value_to_array, 0);
  rb_define_method(cPhpEmbedValue, "to_h", php_value_to_hash, 0);
  rb_define_method(cPhpEmbedValue, "==", php_value_obj_equal, 1);

  rb_define_singleton_method(cPhpEmbedValue, "to_php", php_value_to_php, 1);


}
