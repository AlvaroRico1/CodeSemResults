ALWAYS_INLINE int uca_scanner_900<Mb_wc, LEVELS_FOR_COMPARE>::next() {
  int res = next_raw();
  Coll_param *param = cs->coll_param;
  if (res > 0 && param) {
    /* Reorder weight change only on primary level. */
    if (param->reorder_param && weight_lv == 0) res = apply_reorder_param(res);
    if (param->case_first != CASE_FIRST_OFF) res = apply_case_first(res);
  }
  return res;
}


// Source: ctype-uca.cc
// Lines 1726-1735
