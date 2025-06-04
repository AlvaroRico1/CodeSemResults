lookup_or_add_counter (stats_counter_table_type *hash, const char *id, int val,
		       bool histogram_p)
{
  statistics_counter **counter;
  statistics_counter c;
  c.id = id;
  c.val = val;
  counter = hash->find_slot (&c, INSERT);
  if (!*counter)
    {
      *counter = XNEW (statistics_counter);
      (*counter)->id = xstrdup (id);
      (*counter)->val = val;
      (*counter)->histogram_p = histogram_p;
      (*counter)->prev_dumped_count = 0;
      (*counter)->count = 0;
    }
  return *counter;
}


// Source: statistics.c
// Lines 281-299
