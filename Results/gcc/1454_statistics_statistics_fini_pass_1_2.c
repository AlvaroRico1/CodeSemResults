statistics_fini_pass_1 (statistics_counter **slot,
			void *data ATTRIBUTE_UNUSED)
{
  statistics_counter *counter = *slot;
  unsigned HOST_WIDE_INT count = counter->count - counter->prev_dumped_count;
  if (count == 0)
    return 1;
  if (counter->histogram_p)
    fprintf (dump_file, "%s == %d: " HOST_WIDE_INT_PRINT_DEC "\n",
	     counter->id, counter->val, count);
  else
    fprintf (dump_file, "%s: " HOST_WIDE_INT_PRINT_DEC "\n",
	     counter->id, count);
  counter->prev_dumped_count = counter->count;
  return 1;
}


// Source: statistics.c
// Lines 120-135
