gen_emutls_addr (tree decl, struct lower_emutls_data *d)
{
  /* Compute the address of the TLS variable with help from runtime.  */
  tls_var_data *data = tls_map->get (varpool_node::get (decl));
  tree addr = data->access;

  if (addr == NULL)
    {
      varpool_node *cvar;
      tree cdecl;
      gcall *x;

      cvar = data->control_var;
      cdecl = cvar->decl;
      TREE_ADDRESSABLE (cdecl) = 1;

      addr = create_tmp_var (build_pointer_type (TREE_TYPE (decl)));
      x = gimple_build_call (d->builtin_decl, 1, build_fold_addr_expr (cdecl));
      gimple_set_location (x, d->loc);

      addr = make_ssa_name (addr, x);
      gimple_call_set_lhs (x, addr);

      gimple_seq_add_stmt (&d->seq, x);

      d->cfun_node->create_edge (d->builtin_node, x, d->bb->count);

      /* We may be adding a new reference to a new variable to the function.
         This means we have to play with the ipa-reference web.  */
      d->cfun_node->create_reference (cvar, IPA_REF_ADDR, x);

      /* Record this ssa_name for possible use later in the basic block.  */
      data->access = addr;
    }


// Source: tree-emutls.c
// Lines 397-430
