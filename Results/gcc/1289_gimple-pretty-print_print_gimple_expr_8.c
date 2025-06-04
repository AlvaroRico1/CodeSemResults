print_gimple_expr (FILE *file, gimple *g, int spc, dump_flags_t flags)
{
  flags |= TDF_RHS_ONLY;
  pretty_printer buffer;
  pp_needs_newline (&buffer) = true;
  buffer.buffer->stream = file;
  pp_gimple_stmt_1 (&buffer, g, spc, flags);
  pp_flush (&buffer);
}


// Source: gimple-pretty-print.c
// Lines 182-190
