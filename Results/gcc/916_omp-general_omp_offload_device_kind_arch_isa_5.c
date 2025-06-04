omp_offload_device_kind_arch_isa (const char *props, const char *prop)
{
  const char *names = getenv ("OFFLOAD_TARGET_NAMES");
  if (names == NULL || *names == '\0')
    return false;
  while (*props != '\0')
    {
      size_t name_len = strlen (props);
      bool matches = false;
      for (const char *c = names; c; )
	{
	  if (strncmp (props, c, name_len) == 0
	      && (c[name_len] == '\0'
		  || c[name_len] == ':'
		  || c[name_len] == '='))
	    {
	      matches = true;
	      break;
	    }
	  else if ((c = strchr (c, ':')))
	    c++;
	}
      props = props + name_len + 1;
      while (*props != '\0')
	{
	  if (matches && strcmp (props, prop) == 0)
	    return true;
	  props = strchr (props, '\0') + 1;
	}
      props++;
    }


// Source: omp-general.c
// Lines 649-679
