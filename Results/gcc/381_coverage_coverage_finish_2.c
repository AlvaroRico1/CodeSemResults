coverage_finish (void)
{
  if (bbg_file_name && gcov_close ())
    unlink (bbg_file_name);

  if (!flag_branch_probabilities && flag_test_coverage
      && (!local_tick || local_tick == (unsigned)-1))
    /* Only remove the da file, if we're emitting coverage code and
       cannot uniquely stamp it.  If we can stamp it, libgcov will DTRT.  */
    unlink (da_file_name);

  if (coverage_obj_init ())
    {
      vec<constructor_elt, va_gc> *fn_ctor = NULL;
      struct coverage_data *fn;
      
      for (fn = functions_head; fn; fn = fn->next)
	fn_ctor = coverage_obj_fn (fn_ctor, fn->fn_decl, fn);
      coverage_obj_finish (fn_ctor);
    }


// Source: coverage.c
// Lines 1305-1324
