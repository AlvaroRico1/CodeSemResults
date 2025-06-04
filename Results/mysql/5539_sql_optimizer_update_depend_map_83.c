void JOIN::update_depend_map() {
  ASSERT_BEST_REF_IN_JOIN_ORDER(this);
  for (uint tableno = 0; tableno < tables; tableno++) {
    JOIN_TAB *const tab = best_ref[tableno];
    TABLE_REF *const ref = &tab->ref();
    table_map depend_map = 0;
    Item **item = ref->items;
    for (uint i = 0; i < ref->key_parts; i++, item++)
      depend_map |= (*item)->used_tables();
    depend_map &= ~PSEUDO_TABLE_BITS;
    ref->depend_map = depend_map;
    for (JOIN_TAB **tab2 = map2table; depend_map; tab2++, depend_map >>= 1) {
      if (depend_map & 1) ref->depend_map |= (*tab2)->ref().depend_map;
    }
  }


// Source: sql_optimizer.cc
// Lines 4737-4751
