create_emultls_var (varpool_node *var, void *data)
{
  tree cdecl;
  tls_var_data value;

  cdecl = new_emutls_decl (var->decl,
			   var->alias && var->analyzed
			   ? var->get_alias_target ()->decl : NULL);

  varpool_node *cvar = varpool_node::get (cdecl);

  if (!var->alias)
    {
      /* Make sure the COMMON block control variable gets initialized.
	 Note that there's no point in doing this for aliases; we only
	 need to do this once for the main variable.  */
      emutls_common_1 (var->decl, cdecl, (tree *)data);
    }
  if (var->alias && !var->analyzed)
    cvar->alias = true;

  /* Indicate that the value of the TLS variable may be found elsewhere,
     preventing the variable from re-appearing in the GIMPLE.  We cheat
     and use the control variable here (rather than a full call_expr),
     which is special-cased inside the DWARF2 output routines.  */
  SET_DECL_VALUE_EXPR (var->decl, cdecl);
  DECL_HAS_VALUE_EXPR_P (var->decl) = 1;

  value.control_var = cvar;
  tls_map->put (var, value);

  return false;
}


// Source: tree-emutls.c
// Lines 701-733
