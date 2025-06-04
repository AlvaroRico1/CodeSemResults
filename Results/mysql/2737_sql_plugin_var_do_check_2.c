bool sys_var_pluginvar::do_check(THD *thd, set_var *var) {
  st_item_value_holder value;
  assert(plugin_var->check);

  value.value_type = item_value_type;
  value.val_str = item_val_str;
  value.val_int = item_val_int;
  value.val_real = item_val_real;
  value.is_unsigned = item_is_unsigned;
  value.item = var->value;

  return plugin_var->check(thd, plugin_var, &var->save_result, &value);
}


// Source: sql_plugin_var.cc
// Lines 321-333
