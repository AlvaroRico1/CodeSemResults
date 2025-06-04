dump_access_strides (vec<data_reference_p> datarefs)
{
  data_reference_p dr;
  fprintf (dump_file, "Access Strides for DRs:\n");
  for (unsigned i = 0; datarefs.iterate (i, &dr); ++i)
    {
      fprintf (dump_file, "  ");
      print_generic_expr (dump_file, DR_REF (dr), TDF_SLIM);
      fprintf (dump_file, ":\t\t<");

      vec<tree> *stride = DR_ACCESS_STRIDE (dr);
      unsigned num_loops = stride->length ();
      for (unsigned j = 0; j < num_loops; ++j)
	{
	  print_generic_expr (dump_file, (*stride)[j], TDF_SLIM);
	  fprintf (dump_file, "%s", (j < num_loops - 1) ? ",\t" : ">\n");
	}
    }


// Source: gimple-loop-interchange.cc
// Lines 1417-1434
