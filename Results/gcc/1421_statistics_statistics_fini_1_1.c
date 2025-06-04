statistics_fini_1 (statistics_counter **slot, opt_pass *pass)
{
  statistics_counter *counter = *slot;
  if (counter->count == 0)
    return 1;
  if (counter->histogram_p)
    fprintf (statistics_dump_file,
	     "%d %s \"%s == %d\" " HOST_WIDE_INT_PRINT_DEC "\n",
	     pass->static_pass_number,
	     pass->name,
	     counter->id, counter->val,
	     counter->count);
  else
    fprintf (statistics_dump_file,
	     "%d %s \"%s\" " HOST_WIDE_INT_PRINT_DEC "\n",
	     pass->static_pass_number,
	     pass->name,
	     counter->id,
	     counter->count);
  return 1;
}


// Source: statistics.c
// Lines 210-230
