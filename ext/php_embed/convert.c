#include "value.h"
#include "convert.h"


VALUE zval_to_array(zval* zv) {
  HashTable* ht;
  HashPosition pos;
  zval** data;
  VALUE ret;

  convert_to_array(zv);
  ht = Z_ARRVAL_P(zv);  

  ret = rb_ary_new2(zend_hash_num_elements(ht));

  zend_hash_internal_pointer_reset_ex(ht, &pos);
  while (SUCCESS == zend_hash_get_current_data_ex(ht, (void **)&data, &pos)) {
    rb_ary_push(ret, new_php_embed_value(*data));
    zend_hash_move_forward_ex(ht, &pos);
  }
  return ret;
}

VALUE zval_to_hash(zval* zv) {
  HashTable* ht;
  HashPosition pos;
  zval** data;
  VALUE ret;

  convert_to_array(zv);
  ht = Z_ARRVAL_P(zv);  

  ret = rb_hash_new();

  zend_hash_internal_pointer_reset_ex(ht, &pos);
  while (SUCCESS == zend_hash_get_current_data_ex(ht, (void **)&data, &pos)) {
    char* key_str;
    uint key_len;
    ulong num_index;
    VALUE key = Qnil;
    VALUE val = new_php_embed_value(*data);

    switch(zend_hash_get_current_key_ex(ht, &key_str, &key_len, &num_index, 0, &pos)) {
      case HASH_KEY_IS_STRING:
        //key = rb_str_new(key_str, key_len);
        key = rb_str_new_cstr(key_str);
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

  k = convert_value_to_php_string(key);
  r = rb_str_new_cstr(RSTRING_PTR(k));
  rb_str_cat2(r, "=>");
  v = convert_value_to_php_string(value);
  rb_str_cat2(r, RSTRING_PTR(v));

  rb_ary_push(ary, r);
  return ST_CONTINUE;
}


static int hash_to_zval(VALUE key, VALUE value, VALUE zval_array) {
  zval *v;
  zval *ary;

  if (key == Qundef) { 
    return ST_CONTINUE;
  }

  switch (TYPE(key)) {
    case T_SYMBOL:
      key = rb_sym_to_s(key);
      break;
    case T_FIXNUM:
    case T_BIGNUM:
    case T_STRING:
      key = rb_string_value(&key);
      break;
    default:
      rb_raise(rb_eRuntimeError, "invalid key (%d)", TYPE(key));
  }

  ary = (zval*)zval_array;
  v = value_to_zval(value);
  
  zend_symtable_update(Z_ARRVAL_P(ary), RSTRING_PTR(key), RSTRING_LEN(key), &v, sizeof(zval *), NULL);

  return ST_CONTINUE;
}


VALUE convert_value_to_php_string(VALUE v) {
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
          VALUE p = convert_value_to_php_string(RARRAY_PTR(v)[i]);
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

zval* value_to_zval(VALUE v) {
  zval* zv;
  MAKE_STD_ZVAL(zv);

  switch (TYPE(v)) {
    case T_FALSE:
      ZVAL_FALSE(zv);
      return zv;
    case T_TRUE:
      ZVAL_TRUE(zv);
      return zv;
    case T_UNDEF:
    case T_NIL:
      ZVAL_NULL(zv);
      return zv;
    case T_FIXNUM:
      ZVAL_LONG(zv, rb_fix2int(v));
      return zv;
    case T_BIGNUM:
      ZVAL_LONG(zv, rb_big2long(v)); // FIXME: bignum over long
      return zv;
    case T_FLOAT:
      ZVAL_DOUBLE(zv, RFLOAT_VALUE(v));
      return zv;
    case T_ARRAY:
      {
        int i;
        array_init(zv);
        for(i=0;i<RARRAY_LEN(v);++i) {
          zval *add = value_to_zval(RARRAY_PTR(v)[i]);
          zend_hash_next_index_insert(Z_ARRVAL_P(zv), &add, sizeof(zval*), NULL);
        }
        return zv;
      }
    case T_HASH:
      {
        array_init(zv);
        rb_hash_foreach(v, hash_to_zval, (VALUE)zv);
        return zv;
      }
    case T_SYMBOL:
      {
        VALUE symbol_str = rb_sym_to_s(v);
        ZVAL_STRINGL(zv, StringValuePtr(symbol_str), RSTRING_LEN(symbol_str), 1);
        return zv;
      }
    case T_STRING:
      {
        ZVAL_STRINGL(zv, StringValuePtr(v), RSTRING_LEN(v), 1);
        return zv;
      } 
    default:
      rb_raise(rb_eRuntimeError, "no implemented");
  }
}

