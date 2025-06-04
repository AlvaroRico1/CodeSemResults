build_asm_expr (location_t loc, tree string, tree outputs, tree inputs,
		tree clobbers, tree labels, bool simple, bool is_inline)
{
  tree tail;
  tree args;
  int i;
  const char *constraint;
  const char **oconstraints;
  bool allows_mem, allows_reg, is_inout;
  int ninputs, noutputs;

  ninputs = list_length (inputs);
  noutputs = list_length (outputs);
  oconstraints = (const char **) alloca (noutputs * sizeof (const char *));

  string = resolve_asm_operand_names (string, outputs, inputs, labels);

  /* Remove output conversions that change the type but not the mode.  */
  for (i = 0, tail = outputs; tail; ++i, tail = TREE_CHAIN (tail))
    {
      tree output = TREE_VALUE (tail);

      output = c_fully_fold (output, false, NULL, true);

      /* ??? Really, this should not be here.  Users should be using a
	 proper lvalue, dammit.  But there's a long history of using casts
	 in the output operands.  In cases like longlong.h, this becomes a
	 primitive form of typechecking -- if the cast can be removed, then
	 the output operand had a type of the proper width; otherwise we'll
	 get an error.  Gross, but ...  */
      STRIP_NOPS (output);

      if (!lvalue_or_else (loc, output, lv_asm))
	output = error_mark_node;

      if (output != error_mark_node
	  && (TREE_READONLY (output)
	      || TYPE_READONLY (TREE_TYPE (output))
	      || (RECORD_OR_UNION_TYPE_P (TREE_TYPE (output))
		  && C_TYPE_FIELDS_READONLY (TREE_TYPE (output)))))
	readonly_error (loc, output, lv_asm);

      constraint = TREE_STRING_POINTER (TREE_VALUE (TREE_PURPOSE (tail)));
      oconstraints[i] = constraint;

      if (parse_output_constraint (&constraint, i, ninputs, noutputs,
				   &allows_mem, &allows_reg, &is_inout))
	{
	  /* If the operand is going to end up in memory,
	     mark it addressable.  */
	  if (!allows_reg && !c_mark_addressable (output))
	    output = error_mark_node;
	  if (!(!allows_reg && allows_mem)
	      && output != error_mark_node
	      && VOID_TYPE_P (TREE_TYPE (output)))
	    {
	      error_at (loc, "invalid use of void expression");
	      output = error_mark_node;
	    }
	}
      else
	output = error_mark_node;

      TREE_VALUE (tail) = output;
    }

  for (i = 0, tail = inputs; tail; ++i, tail = TREE_CHAIN (tail))
    {
      tree input;

      constraint = TREE_STRING_POINTER (TREE_VALUE (TREE_PURPOSE (tail)));
      input = TREE_VALUE (tail);

      if (parse_input_constraint (&constraint, i, ninputs, noutputs, 0,
				  oconstraints, &allows_mem, &allows_reg))
	{
	  /* If the operand is going to end up in memory,
	     mark it addressable.  */
	  if (!allows_reg && allows_mem)
	    {
	      input = c_fully_fold (input, false, NULL, true);

	      /* Strip the nops as we allow this case.  FIXME, this really
		 should be rejected or made deprecated.  */
	      STRIP_NOPS (input);
	      if (!c_mark_addressable (input))
		input = error_mark_node;
	    }
	  else
	    {
	      struct c_expr expr;
	      memset (&expr, 0, sizeof (expr));
	      expr.value = input;
	      expr = convert_lvalue_to_rvalue (loc, expr, true, false);
	      input = c_fully_fold (expr.value, false, NULL);

	      if (input != error_mark_node && VOID_TYPE_P (TREE_TYPE (input)))
		{
		  error_at (loc, "invalid use of void expression");
		  input = error_mark_node;
		}
	    }
	}
      else
	input = error_mark_node;

      TREE_VALUE (tail) = input;
    }

  /* ASMs with labels cannot have outputs.  This should have been
     enforced by the parser.  */
  gcc_assert (outputs == NULL || labels == NULL);

  args = build_stmt (loc, ASM_EXPR, string, outputs, inputs, clobbers, labels);

  /* asm statements without outputs, including simple ones, are treated
     as volatile.  */
  ASM_INPUT_P (args) = simple;
  ASM_VOLATILE_P (args) = (noutputs == 0);
  ASM_INLINE_P (args) = is_inline;

  return args;
}


// Source: c-typeck.c
// Lines 10488-10610
