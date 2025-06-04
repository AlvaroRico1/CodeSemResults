static bool propagate_cond_constants(THD *thd, I_List<COND_CMP> *save_list,
                                     Item *and_father, Item *cond) {
  assert(cond->real_item()->is_bool_func());
  if (cond->type() == Item::COND_ITEM) {
    Item_cond *const item_cond = down_cast<Item_cond *>(cond);
    bool and_level = item_cond->functype() == Item_func::COND_AND_FUNC;
    List_iterator_fast<Item> li(*item_cond->argument_list());
    Item *item;
    I_List<COND_CMP> save;
    while ((item = li++)) {
      if (propagate_cond_constants(thd, &save, and_level ? cond : item, item))
        return true;
    }
    if (and_level) {  // Handle other found items
      I_List_iterator<COND_CMP> cond_itr(save);
      COND_CMP *cond_cmp;
      while ((cond_cmp = cond_itr++)) {
        Item **args = cond_cmp->cmp_func->arguments();
        if (!args[0]->const_item() &&
            change_cond_ref_to_const(thd, &save, cond_cmp->and_level,
                                     cond_cmp->and_level, args[0], args[1]))
          return true;
      }
    }
  } else if (and_father != cond &&
             cond->marker != Item::MARKER_CONST_PROPAG)  // In a AND group
  {
    Item_func *func;
    if (cond->type() == Item::FUNC_ITEM &&
        (func = down_cast<Item_func *>(cond)) &&
        (func->functype() == Item_func::EQ_FUNC ||
         func->functype() == Item_func::EQUAL_FUNC)) {
      Item **args = func->arguments();
      bool left_const = args[0]->const_item();
      bool right_const = args[1]->const_item();
      if (!(left_const && right_const) &&
          args[0]->result_type() == args[1]->result_type()) {
        if (right_const) {
          Item *item = args[1];
          if (resolve_const_item(thd, &item, args[0])) return true;
          thd->change_item_tree(&args[1], item);
          func->update_used_tables();
          if (change_cond_ref_to_const(thd, save_list, and_father, and_father,
                                       args[0], args[1]))
            return true;
        } else if (left_const) {
          Item *item = args[0];
          if (resolve_const_item(thd, &item, args[1])) return true;
          thd->change_item_tree(&args[0], item);
          func->update_used_tables();
          if (change_cond_ref_to_const(thd, save_list, and_father, and_father,
                                       args[1], args[0]))
            return true;
        }


// Source: sql_optimizer.cc
// Lines 4615-4668
