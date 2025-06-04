strip_trailing_whitespace (char *desc)
{
  char *terminator = desc + strlen (desc);
  while (desc < terminator)
    {
      terminator--;
      if (ISSPACE (*terminator))
	*terminator = '\0';
      else
	break;
    }
}


// Source: read-rtl-function.c
// Lines 307-318
