validate_equiv_mem_from_store (rtx dest, const_rtx set ATTRIBUTE_UNUSED,
			       void *data)
{
  struct equiv_mem_data *info = (struct equiv_mem_data *) data;

  if ((REG_P (dest)
       && reg_overlap_mentioned_p (dest, info->equiv_mem))
      || (MEM_P (dest)
	  && anti_dependence (info->equiv_mem, dest)))
    info->equiv_mem_modified = true;
}


// Source: ira.c
// Lines 2930-2940
