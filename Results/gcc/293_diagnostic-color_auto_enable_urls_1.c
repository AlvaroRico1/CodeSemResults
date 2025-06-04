auto_enable_urls ()
{
#ifdef __MINGW32__
  return false;
#else
  const char *term, *colorterm;

  /* First check the terminal is capable of printing color escapes,
     if not URLs won't work either.  */
  if (!should_colorize ())
    return false;

  /* xfce4-terminal is known to not implement URLs at this time.
     Recently new installations (0.8) will safely ignore the URL escape
     sequences, but a large number of legacy installations (0.6.3) print
     garbage when URLs are printed.  Therefore we lose nothing by
     disabling this feature for that specific terminal type.  */
  colorterm = getenv ("COLORTERM");
  if (colorterm && !strcmp (colorterm, "xfce4-terminal"))
    return false;

  /* Old versions of gnome-terminal where URL escapes cause screen
     corruptions set COLORTERM="gnome-terminal", recent versions
     with working URL support set this to "truecolor".  */
  if (colorterm && !strcmp (colorterm, "gnome-terminal"))
    return false;

  /* Since the following checks are less specific than the ones
     above, let GCC_URLS and TERM_URLS override the decision.  */
  if (getenv ("GCC_URLS") || getenv ("TERM_URLS"))
    return true;

  /* In an ssh session the COLORTERM is not there, but TERM=xterm
     can be used as an indication of a incompatible terminal while
     TERM=xterm-256color appears to be a working terminal.  */
  term = getenv ("TERM");
  if (!colorterm && term && !strcmp (term, "xterm"))
    return false;

  /* When logging in a linux over serial line, we see TERM=linux
     and no COLORTERM, it is unlikely that the URL escapes will
     work in that environmen either.  */
  if (!colorterm && term && !strcmp (term, "linux"))
    return false;

  return true;
#endif
}


// Source: diagnostic-color.c
// Lines 278-325
