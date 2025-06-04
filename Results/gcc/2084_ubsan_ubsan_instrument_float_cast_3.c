ubsan_instrument_float_cast (location_t loc, tree type, tree expr)
{
  tree expr_type = TREE_TYPE (expr);
  tree t, tt, fn, min, max;
  machine_mode mode = TYPE_MODE (expr_type);
  int prec = TYPE_PRECISION (type);
  bool uns_p = TYPE_UNSIGNED (type);
  if (loc == UNKNOWN_LOCATION)
    loc = input_location;

  /* Float to integer conversion first truncates toward zero, so
     even signed char c = 127.875f; is not problematic.
     Therefore, we should complain only if EXPR is unordered or smaller
     or equal than TYPE_MIN_VALUE - 1.0 or greater or equal than
     TYPE_MAX_VALUE + 1.0.  */
  if (REAL_MODE_FORMAT (mode)->b == 2)
    {
      /* For maximum, TYPE_MAX_VALUE might not be representable
	 in EXPR_TYPE, e.g. if TYPE is 64-bit long long and
	 EXPR_TYPE is IEEE single float, but TYPE_MAX_VALUE + 1.0 is
	 either representable or infinity.  */
      REAL_VALUE_TYPE maxval = dconst1;
      SET_REAL_EXP (&maxval, REAL_EXP (&maxval) + prec - !uns_p);
      real_convert (&maxval, mode, &maxval);
      max = build_real (expr_type, maxval);

      /* For unsigned, assume -1.0 is always representable.  */
      if (uns_p)
	min = build_minus_one_cst (expr_type);
      else
	{
	  /* TYPE_MIN_VALUE is generally representable (or -inf),
	     but TYPE_MIN_VALUE - 1.0 might not be.  */
	  REAL_VALUE_TYPE minval = dconstm1, minval2;
	  SET_REAL_EXP (&minval, REAL_EXP (&minval) + prec - 1);
	  real_convert (&minval, mode, &minval);
	  real_arithmetic (&minval2, MINUS_EXPR, &minval, &dconst1);
	  real_convert (&minval2, mode, &minval2);
	  if (real_compare (EQ_EXPR, &minval, &minval2)
	      && !real_isinf (&minval))
	    {
	      /* If TYPE_MIN_VALUE - 1.0 is not representable and
		 rounds to TYPE_MIN_VALUE, we need to subtract
		 more.  As REAL_MODE_FORMAT (mode)->p is the number
		 of base digits, we want to subtract a number that
		 will be 1 << (REAL_MODE_FORMAT (mode)->p - 1)
		 times smaller than minval.  */
	      minval2 = dconst1;
	      gcc_assert (prec > REAL_MODE_FORMAT (mode)->p);
	      SET_REAL_EXP (&minval2,
			    REAL_EXP (&minval2) + prec - 1
			    - REAL_MODE_FORMAT (mode)->p + 1);
	      real_arithmetic (&minval2, MINUS_EXPR, &minval, &minval2);
	      real_convert (&minval2, mode, &minval2);
	    }
	  min = build_real (expr_type, minval2);
	}
    }
  else if (REAL_MODE_FORMAT (mode)->b == 10)
    {
      /* For _Decimal128 up to 34 decimal digits, - sign,
	 dot, e, exponent.  */
      char buf[64];
      mpfr_t m;
      int p = REAL_MODE_FORMAT (mode)->p;
      REAL_VALUE_TYPE maxval, minval;

      /* Use mpfr_snprintf rounding to compute the smallest
	 representable decimal number greater or equal than
	 1 << (prec - !uns_p).  */
      mpfr_init2 (m, prec + 2);
      mpfr_set_ui_2exp (m, 1, prec - !uns_p, MPFR_RNDN);
      mpfr_snprintf (buf, sizeof buf, "%.*RUe", p - 1, m);
      decimal_real_from_string (&maxval, buf);
      max = build_real (expr_type, maxval);

      /* For unsigned, assume -1.0 is always representable.  */
      if (uns_p)
	min = build_minus_one_cst (expr_type);
      else
	{
	  /* Use mpfr_snprintf rounding to compute the largest
	     representable decimal number less or equal than
	     (-1 << (prec - 1)) - 1.  */
	  mpfr_set_si_2exp (m, -1, prec - 1, MPFR_RNDN);
	  mpfr_sub_ui (m, m, 1, MPFR_RNDN);
	  mpfr_snprintf (buf, sizeof buf, "%.*RDe", p - 1, m);
	  decimal_real_from_string (&minval, buf);
	  min = build_real (expr_type, minval);
	}
      mpfr_clear (m);
    }
  else
    return NULL_TREE;

  t = fold_build2 (UNLE_EXPR, boolean_type_node, expr, min);
  tt = fold_build2 (UNGE_EXPR, boolean_type_node, expr, max);
  t = fold_build2 (TRUTH_OR_EXPR, boolean_type_node, t, tt);
  if (integer_zerop (t))
    return NULL_TREE;

  if (flag_sanitize_undefined_trap_on_error)
    fn = build_call_expr_loc (loc, builtin_decl_explicit (BUILT_IN_TRAP), 0);
  else
    {
      location_t *loc_ptr = NULL;
      unsigned num_locations = 0;
      /* Figure out if we can propagate location to ubsan_data and use new
         style handlers in libubsan.  */
      if (ubsan_use_new_style_p (loc))
	{
	  loc_ptr = &loc;
	  num_locations = 1;
	}
      /* Create the __ubsan_handle_float_cast_overflow fn call.  */
      tree data = ubsan_create_data ("__ubsan_float_cast_overflow_data",
				     num_locations, loc_ptr,
				     ubsan_type_descriptor (expr_type),
				     ubsan_type_descriptor (type), NULL_TREE,
				     NULL_TREE);
      enum built_in_function bcode
	= (flag_sanitize_recover & SANITIZE_FLOAT_CAST)
	  ? BUILT_IN_UBSAN_HANDLE_FLOAT_CAST_OVERFLOW
	  : BUILT_IN_UBSAN_HANDLE_FLOAT_CAST_OVERFLOW_ABORT;
      fn = builtin_decl_explicit (bcode);
      fn = build_call_expr_loc (loc, fn, 2,
				build_fold_addr_expr_loc (loc, data),
				ubsan_encode_value (expr));
    }


// Source: ubsan.c
// Lines 1795-1923
