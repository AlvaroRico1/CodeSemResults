copy_rtx_if_shared_1 (rtx *orig1)
{
  rtx x;
  int i;
  enum rtx_code code;
  rtx *last_ptr;
  const char *format_ptr;
  int copied = 0;
  int length;

  /* Repeat is used to turn tail-recursion into iteration.  */
repeat:
  x = *orig1;

  if (x == 0)
    return;

  code = GET_CODE (x);

  /* These types may be freely shared.  */

  switch (code)
    {
    case REG:
    case DEBUG_EXPR:
    case VALUE:
    CASE_CONST_ANY:
    case SYMBOL_REF:
    case LABEL_REF:
    case CODE_LABEL:
    case PC:
    case CC0:
    case RETURN:
    case SIMPLE_RETURN:
    case SCRATCH:
      /* SCRATCH must be shared because they represent distinct values.  */
      return;
    case CLOBBER:
      /* Share clobbers of hard registers (like cc0), but do not share pseudo reg
         clobbers or clobbers of hard registers that originated as pseudos.
         This is needed to allow safe register renaming.  */
      if (REG_P (XEXP (x, 0))
	  && HARD_REGISTER_NUM_P (REGNO (XEXP (x, 0)))
	  && HARD_REGISTER_NUM_P (ORIGINAL_REGNO (XEXP (x, 0))))
	return;
      break;

    case CONST:
      if (shared_const_p (x))
	return;
      break;

    case DEBUG_INSN:
    case INSN:
    case JUMP_INSN:
    case CALL_INSN:
    case NOTE:
    case BARRIER:
      /* The chain of insns is not being copied.  */
      return;

    default:
      break;
    }

  /* This rtx may not be shared.  If it has already been seen,
     replace it with a copy of itself.  */

  if (RTX_FLAG (x, used))
    {
      x = shallow_copy_rtx (x);
      copied = 1;
    }
  RTX_FLAG (x, used) = 1;

  /* Now scan the subexpressions recursively.
     We can store any replaced subexpressions directly into X
     since we know X is not shared!  Any vectors in X
     must be copied if X was copied.  */

  format_ptr = GET_RTX_FORMAT (code);
  length = GET_RTX_LENGTH (code);
  last_ptr = NULL;

  for (i = 0; i < length; i++)
    {
      switch (*format_ptr++)
	{
	case 'e':
          if (last_ptr)
            copy_rtx_if_shared_1 (last_ptr);
	  last_ptr = &XEXP (x, i);
	  break;

	case 'E':
	  if (XVEC (x, i) != NULL)
	    {
	      int j;
	      int len = XVECLEN (x, i);

              /* Copy the vector iff I copied the rtx and the length
		 is nonzero.  */
	      if (copied && len > 0)
		XVEC (x, i) = gen_rtvec_v (len, XVEC (x, i)->elem);

              /* Call recursively on all inside the vector.  */
	      for (j = 0; j < len; j++)
                {
		  if (last_ptr)
		    copy_rtx_if_shared_1 (last_ptr);
                  last_ptr = &XVECEXP (x, i, j);
                }
	    }
	  break;
	}
    }
  *orig1 = x;
  if (last_ptr)
    {
      orig1 = last_ptr;
      goto repeat;
    }
  return;
}


// Source: emit-rtl.c
// Lines 3070-3193
