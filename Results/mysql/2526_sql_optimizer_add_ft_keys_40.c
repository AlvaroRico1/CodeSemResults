static bool add_ft_keys(Key_use_array *keyuse_array, Item *cond,
                        table_map usable_tables, bool simple_match_expr) {
  Item_func_match *cond_func = nullptr;

  if (!cond) return false;

  assert(cond->is_bool_func());

  if (cond->type() == Item::FUNC_ITEM) {
    Item_func *func = down_cast<Item_func *>(cond);
    Item_func::Functype functype = func->functype();
    if (functype == Item_func::MATCH_FUNC) {
      func = down_cast<Item_func *>(func->arguments()[0]);
      functype = func->functype();
    }
    enum ft_operation op_type = FT_OP_NO;
    double op_value = 0.0;
    if (functype == Item_func::FT_FUNC) {
      cond_func = down_cast<Item_func_match *>(func)->get_master();
      cond_func->set_hints_op(op_type, op_value);
    } else if (func->arg_count == 2) {
      Item *arg0 = func->arguments()[0];
      Item *arg1 = func->arguments()[1];
      if (arg1->const_item() && arg0->type() == Item::FUNC_ITEM &&
          down_cast<Item_func *>(arg0)->functype() == Item_func::FT_FUNC &&
          ((functype == Item_func::GE_FUNC &&
            (op_value = arg1->val_real()) > 0) ||
           (functype == Item_func::GT_FUNC &&
            (op_value = arg1->val_real()) >= 0))) {
        cond_func = down_cast<Item_func_match *>(arg0)->get_master();
        if (functype == Item_func::GE_FUNC)
          op_type = FT_OP_GE;
        else if (functype == Item_func::GT_FUNC)
          op_type = FT_OP_GT;
        cond_func->set_hints_op(op_type, op_value);
      } else if (arg0->const_item() && arg1->type() == Item::FUNC_ITEM &&
                 down_cast<Item_func *>(arg1)->functype() ==
                     Item_func::FT_FUNC &&
                 ((functype == Item_func::LE_FUNC &&
                   (op_value = arg0->val_real()) > 0) ||
                  (functype == Item_func::LT_FUNC &&
                   (op_value = arg0->val_real()) >= 0))) {
        cond_func = down_cast<Item_func_match *>(arg1)->get_master();
        if (functype == Item_func::LE_FUNC)
          op_type = FT_OP_GE;
        else if (functype == Item_func::LT_FUNC)
          op_type = FT_OP_GT;
        cond_func->set_hints_op(op_type, op_value);
      }
    }
  } else if (cond->type() == Item::COND_ITEM) {
    List_iterator_fast<Item> li(*down_cast<Item_cond *>(cond)->argument_list());

    if (down_cast<Item_cond *>(cond)->functype() == Item_func::COND_AND_FUNC) {
      Item *item;
      while ((item = li++))
        if (add_ft_keys(keyuse_array, item, usable_tables, false)) return true;
    }
  }

  if (!cond_func || cond_func->key == NO_SUCH_KEY ||
      !(usable_tables & cond_func->table_ref->map()))
    return false;

  TABLE_LIST *tbl = cond_func->table_ref;
  if (!tbl->table->keys_in_use_for_query.is_set(cond_func->key)) return false;

  cond_func->set_simple_expression(simple_match_expr);

  const Key_use keyuse(tbl, cond_func, cond_func->key_item()->used_tables(),
                       cond_func->key, FT_KEYPART,
                       0,            // optimize
                       0,            // keypart_map
                       ~(ha_rows)0,  // ref_table_rows
                       false,        // null_rejecting
                       nullptr,      // cond_guard
                       UINT_MAX);    // sj_pred_no
  tbl->table->reginfo.join_tab->keys().set_bit(cond_func->key);
  return keyuse_array->push_back(keyuse);
}


// Source: sql_optimizer.cc
// Lines 7461-7540
