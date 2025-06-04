gimple_dump_bb (FILE *file, basic_block bb, int indent, dump_flags_t flags)
{
  dump_gimple_bb_header (file, bb, indent, flags);
  if (bb->index >= NUM_FIXED_BLOCKS)
    {
      pretty_printer buffer;
      pp_needs_newline (&buffer) = true;
      buffer.buffer->stream = file;
      gimple_dump_bb_buff (&buffer, bb, indent, flags);
    }


// Source: gimple-pretty-print.c
// Lines 2980-2989
