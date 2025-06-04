graphviz_out::print (const char *fmt, ...)
{
  text_info text;
  va_list ap;

  va_start (ap, fmt);
  text.err_no = errno;
  text.args_ptr = &ap;
  text.format_spec = fmt;
  pp_format (m_pp, &text);
  pp_output_formatted_text (m_pp);
  va_end (ap);
}


// Source: graphviz.cc
// Lines 37-49
