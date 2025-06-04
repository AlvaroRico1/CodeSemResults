dump_class (struct table_elt *classp)
{
  struct table_elt *elt;

  fprintf (stderr, "Equivalence chain for ");
  print_rtl (stderr, classp->exp);
  fprintf (stderr, ": \n");

  for (elt = classp->first_same_value; elt; elt = elt->next_same_value)
    {
      print_rtl (stderr, elt->exp);
      fprintf (stderr, "\n");
    }
}


// Source: cse.c
// Lines 634-647
