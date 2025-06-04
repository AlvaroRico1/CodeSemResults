dump_ssaname_info_to_file (FILE *file, tree node, int spc)
{
  pretty_printer buffer;
  pp_needs_newline (&buffer) = true;
  buffer.buffer->stream = file;
  dump_ssaname_info (&buffer, node, spc);
  pp_flush (&buffer);
}


// Source: gimple-pretty-print.c
// Lines 2276-2283
