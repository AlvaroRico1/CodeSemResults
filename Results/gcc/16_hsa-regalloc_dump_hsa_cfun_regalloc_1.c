dump_hsa_cfun_regalloc (FILE *f)
{
  basic_block bb;

  fprintf (f, "\nHSAIL IL for %s\n", hsa_cfun->m_name);

  FOR_ALL_BB_FN (bb, cfun)
  {
    hsa_bb *hbb = (class hsa_bb *) bb->aux;
    bitmap_print (dump_file, hbb->m_livein, "m_livein  ", "\n");
    dump_hsa_bb (f, hbb);
    bitmap_print (dump_file, hbb->m_liveout, "m_liveout ", "\n");
  }


// Source: hsa-regalloc.c
// Lines 252-264
