static void swap_parameter_array(Item_param **param_array_dst,
                                 Item_param **param_array_src,
                                 uint param_count) {
  Item_param **dst = param_array_dst;
  Item_param **src = param_array_src;
  Item_param **end = param_array_dst + param_count;

  for (; dst < end; ++src, ++dst) {
    (*dst)->set_param_type_and_swap_value(*src);
    (*dst)->sync_clones();
    (*src)->sync_clones();
  }
}


// Source: sql_prepare.cc
// Lines 908-920
