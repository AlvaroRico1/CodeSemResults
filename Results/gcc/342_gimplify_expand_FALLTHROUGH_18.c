expand_FALLTHROUGH (gimple_seq *seq_p)
{
  struct walk_stmt_info wi;
  location_t loc;
  memset (&wi, 0, sizeof (wi));
  wi.info = (void *) &loc;
  walk_gimple_seq_mod (seq_p, expand_FALLTHROUGH_r, NULL, &wi);
  if (wi.callback_result == integer_zero_node)
    /* We've found [[fallthrough]]; at the end of a switch, which the C++
       standard says is ill-formed; see [dcl.attr.fallthrough].  */
    pedwarn (loc, 0, "attribute %<fallthrough%> not preceding "
	     "a case label or default label");
}


// Source: gimplify.c
// Lines 2437-2449
