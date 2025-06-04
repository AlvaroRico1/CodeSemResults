statistics_fini_pass_3 (statistics_counter **slot,
			void *data ATTRIBUTE_UNUSED)
{
  statistics_counter *counter = *slot;
  counter->prev_dumped_count = counter->count;
  return 1;
}


// Source: statistics.c
// Lines 172-178
