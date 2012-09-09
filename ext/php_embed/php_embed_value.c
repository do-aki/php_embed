#include <ruby.h>
#include <php.h>
#include <php_embed_value.h>

static void php_value_mark(php_value* pv) {
  if (pv) {
    rb_gc_mark(pv->value);
  }
}

static void php_value_free(php_value* pv) {
  if (pv) {
    xfree(pv);
  }
}

static VALUE php_value_alloc(VALUE klass) {
  php_value * pv;
  return Data_Make_Struct(klass, php_value, php_value_mark, php_value_free, pv);
}

static int hash_to_php_string_array(VALUE key, VALUE value, VALUE ary) {
    VALUE k,v,r;
    int key_type;

    if (key == Qundef) { 
      return ST_CONTINUE;
    }

    key_type = TYPE(key);
    if (T_FIXNUM != key_type && T_BIGNUM != key_type 
        && T_STRING != key_type && T_SYMBOL != key_type) {
      rb_raise(rb_eRuntimeError, "invalid key (%d)", key_type);
    }

    k = convert_value_into_php_string(key);
    r = rb_str_new_cstr(RSTRING_PTR(k));
    rb_str_cat2(r, "=>");
    v = convert_value_into_php_string(value);
    rb_str_cat2(r, RSTRING_PTR(v));

    rb_ary_push(ary, r);
    return ST_CONTINUE;
}

VALUE convert_value_into_php_string(VALUE v) {
  switch (TYPE(v)) {
    case T_FALSE:
      return rb_str_new_cstr("false");
    case T_TRUE:
      return rb_str_new_cstr("true");
    case T_UNDEF:
    case T_NIL:
      return rb_str_new_cstr("null");
    case T_FIXNUM:
      return rb_fix2str(v, 10);
    case T_BIGNUM:
      return rb_big2str(v, 10);
    case T_FLOAT:
      return rb_funcall(v, rb_intern("to_s"), 0);
    case T_ARRAY:
      {
        int i;
        VALUE ret = rb_str_new_cstr("array(");
        for(i=0;i<RARRAY_LEN(v);++i) {
          VALUE p = convert_value_into_php_string(RARRAY_PTR(v)[i]);
          if (T_STRING == TYPE(p)) {
            rb_str_cat2(ret, StringValuePtr(p));
          }
          if (i != RARRAY_LEN(v)-1) {
            rb_str_cat2(ret, ",");
          }
        }
        rb_str_cat2(ret, ")");
        return ret;
      }
    case T_HASH:
      {
        VALUE ret = rb_str_new_cstr("array(");
        VALUE ary = rb_ary_new();

        rb_hash_foreach(v, hash_to_php_string_array, ary);
        
        {
          int i;
          int len = RARRAY_LEN(ary);

          VALUE* p = RARRAY_PTR(ary);
          for(i=0; i<len; ++i) {
            rb_str_cat2(ret, StringValuePtr(p[i]));
            if (i != len-1) {
              rb_str_cat2(ret, ",");
            }
          }
        }
        rb_str_cat2(ret, ")");
        return ret;
      }
    case T_SYMBOL:
      {
        VALUE symbol_str = rb_sym_to_s(v);
        VALUE ret = rb_str_new_cstr("'");
        rb_str_cat2(ret, StringValuePtr(symbol_str));
        rb_str_cat2(ret, "'");
        return ret;
      }
    case T_STRING:
      {
        VALUE ret = rb_str_new_cstr("'");
        rb_str_cat2(ret, StringValuePtr(v));
        rb_str_cat2(ret, "'");
        return ret;
      } 
    default:
      rb_raise(rb_eRuntimeError, "no implemented");
  }
}


VALUE php_value_initialize(VALUE self, VALUE value) {
  php_value* pv;
  Data_Get_Struct(self, php_value, pv);

  pv->value = value;
  return Qnil;
}

VALUE php_value_to_string(VALUE self) {
  VALUE ret;
  php_value* pv;
  Data_Get_Struct(self, php_value, pv);

  return convert_value_into_php_string(pv->value);
}

VALUE php_value_parse_php_string(VALUE self, VALUE php_string) {
  return Qnil;
}


extern mPhpEmbed;
VALUE cPhpEmbedValue;
void init_php_value() {
#if 0
  mPhpEmbed = rb_define_module("PhpEmbed");
#endif

  cPhpEmbedValue = rb_define_class_under(mPhpEmbed, "Value", rb_cObject);
  rb_define_alloc_func(cPhpEmbedValue, php_value_alloc);
  rb_define_method(cPhpEmbedValue, "initialize", php_value_initialize, 1);
  rb_define_method(cPhpEmbedValue, "to_s", php_value_to_string, 0);

  rb_define_singleton_method(cPhpEmbedValue, "parse", php_value_parse_php_string, 1);


}
