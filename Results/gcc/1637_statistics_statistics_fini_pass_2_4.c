statistics_fini_pass_2 (statistics_counter **slot,
			void *data ATTRIBUTE_UNUSED)
{
  statistics_counter *counter = *slot;
  unsigned HOST_WIDE_INT count = counter->count - counter->prev_dumped_count;
  if (count == 0)
    return 1;
  counter->prev_dumped_count = counter->count;
  if (counter->histogram_p)
    fprintf (statistics_dump_file,
	     "%d %s \"%s == %d\" \"%s\" " HOST_WIDE_INT_PRINT_DEC "\n",
	     current_pass->static_pass_number,
	     current_pass->name,
	     counter->id, counter->val,
	     current_function_name (),
	     count);
  else
    fprintf (statistics_dump_file,
	     "%d %s \"%s\" \"%s\" " HOST_WIDE_INT_PRINT_DEC "\n",
	     current_pass->static_pass_number,
	     current_pass->name,
	     counter->id,
	     current_function_name (),
	     count);
  counter->prev_dumped_count = counter->count;
  return 1;
}


// Source: statistics.c
// Lines 141-167
