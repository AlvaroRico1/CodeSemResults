instrument_bool_enum_load (gimple_stmt_iterator *gsi)
{
  gimple *stmt = gsi_stmt (*gsi);
  tree rhs = gimple_assign_rhs1 (stmt);
  tree type = TREE_TYPE (rhs);
  tree minv = NULL_TREE, maxv = NULL_TREE;

  if (TREE_CODE (type) == BOOLEAN_TYPE
      && sanitize_flags_p (SANITIZE_BOOL))
    {
      minv = boolean_false_node;
      maxv = boolean_true_node;
    }
  else if (TREE_CODE (type) == ENUMERAL_TYPE
	   && sanitize_flags_p (SANITIZE_ENUM)
	   && TREE_TYPE (type) != NULL_TREE
	   && TREE_CODE (TREE_TYPE (type)) == INTEGER_TYPE
	   && (TYPE_PRECISION (TREE_TYPE (type))
	       < GET_MODE_PRECISION (SCALAR_INT_TYPE_MODE (type))))
    {
      minv = TYPE_MIN_VALUE (TREE_TYPE (type));
      maxv = TYPE_MAX_VALUE (TREE_TYPE (type));
    }
  else
    return;

  int modebitsize = GET_MODE_BITSIZE (SCALAR_INT_TYPE_MODE (type));
  poly_int64 bitsize, bitpos;
  tree offset;
  machine_mode mode;
  int volatilep = 0, reversep, unsignedp = 0;
  tree base = get_inner_reference (rhs, &bitsize, &bitpos, &offset, &mode,
				   &unsignedp, &reversep, &volatilep);
  tree utype = build_nonstandard_integer_type (modebitsize, 1);

  if ((VAR_P (base) && DECL_HARD_REGISTER (base))
      || !multiple_p (bitpos, modebitsize)
      || maybe_ne (bitsize, modebitsize)
      || GET_MODE_BITSIZE (SCALAR_INT_TYPE_MODE (utype)) != modebitsize
      || TREE_CODE (gimple_assign_lhs (stmt)) != SSA_NAME)
    return;

  bool ends_bb = stmt_ends_bb_p (stmt);
  location_t loc = gimple_location (stmt);
  tree lhs = gimple_assign_lhs (stmt);
  tree ptype = build_pointer_type (TREE_TYPE (rhs));
  tree atype = reference_alias_ptr_type (rhs);
  gimple *g = gimple_build_assign (make_ssa_name (ptype),
				  build_fold_addr_expr (rhs));
  gimple_set_location (g, loc);
  gsi_insert_before (gsi, g, GSI_SAME_STMT);
  tree mem = build2 (MEM_REF, utype, gimple_assign_lhs (g),
		     build_int_cst (atype, 0));
  tree urhs = make_ssa_name (utype);
  if (ends_bb)
    {
      gimple_assign_set_lhs (stmt, urhs);
      g = gimple_build_assign (lhs, NOP_EXPR, urhs);
      gimple_set_location (g, loc);
      edge e = find_fallthru_edge (gimple_bb (stmt)->succs);
      gsi_insert_on_edge_immediate (e, g);
      gimple_assign_set_rhs_from_tree (gsi, mem);
      update_stmt (stmt);
      *gsi = gsi_for_stmt (g);
      g = stmt;
    }
  else
    {
      g = gimple_build_assign (urhs, mem);
      gimple_set_location (g, loc);
      gsi_insert_before (gsi, g, GSI_SAME_STMT);
    }
  minv = fold_convert (utype, minv);
  maxv = fold_convert (utype, maxv);
  if (!integer_zerop (minv))
    {
      g = gimple_build_assign (make_ssa_name (utype), MINUS_EXPR, urhs, minv);
      gimple_set_location (g, loc);
      gsi_insert_before (gsi, g, GSI_SAME_STMT);
    }

  gimple_stmt_iterator gsi2 = *gsi;
  basic_block then_bb, fallthru_bb;
  *gsi = create_cond_insert_point (gsi, true, false, true,
				   &then_bb, &fallthru_bb);
  g = gimple_build_cond (GT_EXPR, gimple_assign_lhs (g),
			 int_const_binop (MINUS_EXPR, maxv, minv),
			 NULL_TREE, NULL_TREE);
  gimple_set_location (g, loc);
  gsi_insert_after (gsi, g, GSI_NEW_STMT);

  if (!ends_bb)
    {
      gimple_assign_set_rhs_with_ops (&gsi2, NOP_EXPR, urhs);
      update_stmt (stmt);
    }

  gsi2 = gsi_after_labels (then_bb);
  if (flag_sanitize_undefined_trap_on_error)
    g = gimple_build_call (builtin_decl_explicit (BUILT_IN_TRAP), 0);
  else
    {
      tree data = ubsan_create_data ("__ubsan_invalid_value_data", 1, &loc,
				     ubsan_type_descriptor (type), NULL_TREE,
				     NULL_TREE);
      data = build_fold_addr_expr_loc (loc, data);
      enum built_in_function bcode
	= (flag_sanitize_recover & (TREE_CODE (type) == BOOLEAN_TYPE
				    ? SANITIZE_BOOL : SANITIZE_ENUM))
	  ? BUILT_IN_UBSAN_HANDLE_LOAD_INVALID_VALUE
	  : BUILT_IN_UBSAN_HANDLE_LOAD_INVALID_VALUE_ABORT;
      tree fn = builtin_decl_explicit (bcode);

      tree val = ubsan_encode_value (urhs, UBSAN_ENCODE_VALUE_GIMPLE);
      val = force_gimple_operand_gsi (&gsi2, val, true, NULL_TREE, true,
				      GSI_SAME_STMT);
      g = gimple_build_call (fn, 2, data, val);
    }
  gimple_set_location (g, loc);
  gsi_insert_before (&gsi2, g, GSI_SAME_STMT);
  ubsan_create_edge (g);
  *gsi = gsi_for_stmt (stmt);
}


// Source: ubsan.c
// Lines 1643-1765
