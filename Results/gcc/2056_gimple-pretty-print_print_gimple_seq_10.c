print_gimple_seq (FILE *file, gimple_seq seq, int spc, dump_flags_t flags)
{
  pretty_printer buffer;
  pp_needs_newline (&buffer) = true;
  buffer.buffer->stream = file;
  dump_gimple_seq (&buffer, seq, spc, flags);
  pp_newline_and_flush (&buffer);
}


// Source: gimple-pretty-print.c
// Lines 219-226
