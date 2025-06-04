replace_dead_reg (rtx x, const_rtx old_rtx ATTRIBUTE_UNUSED, void *data)
{
  rtx *replacements = (rtx *) data;

  if (REG_P (x)
      && REGNO (x) >= FIRST_PSEUDO_REGISTER
      && replacements[REGNO (x)] != NULL_RTX)
    {
      if (GET_MODE (x) == GET_MODE (replacements[REGNO (x)]))
	return replacements[REGNO (x)];
      return lowpart_subreg (GET_MODE (x), replacements[REGNO (x)],
			     GET_MODE (replacements[REGNO (x)]));
    }
  return NULL_RTX;
}


// Source: cse.c
// Lines 7033-7047
