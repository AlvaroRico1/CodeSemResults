c_cast_expr (location_t loc, struct c_type_name *type_name, tree expr)
{
  tree type;
  tree type_expr = NULL_TREE;
  bool type_expr_const = true;
  tree ret;
  int saved_wsp = warn_strict_prototypes;

  /* This avoids warnings about unprototyped casts on
     integers.  E.g. "#define SIG_DFL (void(*)())0".  */
  if (TREE_CODE (expr) == INTEGER_CST)
    warn_strict_prototypes = 0;
  type = groktypename (type_name, &type_expr, &type_expr_const);
  warn_strict_prototypes = saved_wsp;

  if (TREE_CODE (expr) == ADDR_EXPR && !VOID_TYPE_P (type)
      && reject_gcc_builtin (expr))
    return error_mark_node;

  ret = build_c_cast (loc, type, expr);
  if (type_expr)
    {
      bool inner_expr_const = true;
      ret = c_fully_fold (ret, require_constant_value, &inner_expr_const);
      ret = build2 (C_MAYBE_CONST_EXPR, TREE_TYPE (ret), type_expr, ret);
      C_MAYBE_CONST_EXPR_NON_CONST (ret) = !(type_expr_const
					     && inner_expr_const);
      SET_EXPR_LOCATION (ret, loc);
    }

  if (!EXPR_HAS_LOCATION (ret))
    protected_set_expr_location (ret, loc);

  /* C++ does not permits types to be defined in a cast, but it
     allows references to incomplete types.  */
  if (warn_cxx_compat && type_name->specs->typespec_kind == ctsk_tagdef)
    warning_at (loc, OPT_Wc___compat,
		"defining a type in a cast is invalid in C++");

  return ret;
}


// Source: c-typeck.c
// Lines 6041-6081
