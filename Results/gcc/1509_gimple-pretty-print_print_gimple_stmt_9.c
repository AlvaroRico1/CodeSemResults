print_gimple_stmt (FILE *file, gimple *g, int spc, dump_flags_t flags)
{
  pretty_printer buffer;
  pp_needs_newline (&buffer) = true;
  buffer.buffer->stream = file;
  pp_gimple_stmt_1 (&buffer, g, spc, flags);
  pp_newline_and_flush (&buffer);
}


// Source: gimple-pretty-print.c
// Lines 152-159
