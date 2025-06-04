free_omp_region_1 (struct omp_region *region)
{
  struct omp_region *i, *n;

  for (i = region->inner; i ; i = n)
    {
      n = i->next;
      free_omp_region_1 (i);
    }

  free (region);
}


// Source: omp-expand.c
// Lines 447-458
