coverage_obj_init (void)
{
  tree gcov_info_type;
  unsigned n_counters = 0;
  unsigned ix;
  struct coverage_data *fn;
  struct coverage_data **fn_prev;
  char name_buf[32];

  no_coverage = 1; /* Disable any further coverage.  */

  if (!prg_ctr_mask)
    return false;

  if (symtab->dump_file)
    fprintf (symtab->dump_file, "Using data file %s\n", da_file_name);

  /* Prune functions.  */
  for (fn_prev = &functions_head; (fn = *fn_prev);)
    if (DECL_STRUCT_FUNCTION (fn->fn_decl))
      fn_prev = &fn->next;
    else
      /* The function is not being emitted, remove from list.  */
      *fn_prev = fn->next;

  if (functions_head == NULL)
    return false;

  for (ix = 0; ix != GCOV_COUNTERS; ix++)
    if ((1u << ix) & prg_ctr_mask)
      n_counters++;
  
  /* Build the info and fn_info types.  These are mutually recursive.  */
  gcov_info_type = lang_hooks.types.make_type (RECORD_TYPE);
  gcov_fn_info_type = lang_hooks.types.make_type (RECORD_TYPE);
  build_fn_info_type (gcov_fn_info_type, n_counters, gcov_info_type);
  gcov_info_type = lang_hooks.types.make_type (RECORD_TYPE);
  gcov_fn_info_ptr_type = build_pointer_type
    (build_qualified_type (gcov_fn_info_type, TYPE_QUAL_CONST));
  build_info_type (gcov_info_type, gcov_fn_info_ptr_type);
  
  /* Build the gcov info var, this is referred to in its own
     initializer.  */
  gcov_info_var = build_decl (BUILTINS_LOCATION,
			      VAR_DECL, NULL_TREE, gcov_info_type);
  TREE_STATIC (gcov_info_var) = 1;
  ASM_GENERATE_INTERNAL_LABEL (name_buf, "LPBX", 0);
  DECL_NAME (gcov_info_var) = get_identifier (name_buf);

  build_init_ctor (gcov_info_type);
  build_gcov_exit_decl ();

  return true;
}


// Source: coverage.c
// Lines 1099-1152
