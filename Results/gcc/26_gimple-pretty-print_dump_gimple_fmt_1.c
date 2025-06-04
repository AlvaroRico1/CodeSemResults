dump_gimple_fmt (pretty_printer *buffer, int spc, dump_flags_t flags,
                 const char *fmt, ...)
{
  va_list args;
  const char *c;
  const char *tmp;

  va_start (args, fmt);
  for (c = fmt; *c; c++)
    {
      if (*c == '%')
        {
          gimple_seq seq;
          tree t;
	  gimple *g;
          switch (*++c)
            {
              case 'G':
		g = va_arg (args, gimple *);
                tmp = gimple_code_name[gimple_code (g)];
                pp_string (buffer, tmp);
                break;

              case 'S':
                seq = va_arg (args, gimple_seq);
                pp_newline (buffer);
                dump_gimple_seq (buffer, seq, spc + 2, flags);
                newline_and_indent (buffer, spc);
                break;

              case 'T':
                t = va_arg (args, tree);
                if (t == NULL_TREE)
                  pp_string (buffer, "NULL");
                else
                  dump_generic_node (buffer, t, spc, flags, false);
                break;

              case 'd':
                pp_decimal_int (buffer, va_arg (args, int));
                break;

              case 's':
                pp_string (buffer, va_arg (args, char *));
                break;

              case 'n':
                newline_and_indent (buffer, spc);
                break;

	      case 'x':
		pp_scalar (buffer, "%x", va_arg (args, int));
		break;

              case '+':
                spc += 2;
                newline_and_indent (buffer, spc);
                break;

              case '-':
                spc -= 2;
                newline_and_indent (buffer, spc);
                break;

              default:
                gcc_unreachable ();
            }
        }
      else
        pp_character (buffer, *c);
    }
  va_end (args);
}


// Source: gimple-pretty-print.c
// Lines 251-323
