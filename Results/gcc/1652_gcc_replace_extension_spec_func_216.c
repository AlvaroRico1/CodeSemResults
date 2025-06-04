replace_extension_spec_func (int argc, const char **argv)
{
  char *name;
  char *p;
  char *result;
  int i;

  if (argc != 2)
    fatal_error (input_location, "too few arguments to %%:replace-extension");

  name = xstrdup (argv[0]);

  for (i = strlen (name) - 1; i >= 0; i--)
    if (IS_DIR_SEPARATOR (name[i]))
      break;

  p = strrchr (name + i + 1, '.');
  if (p != NULL)
      *p = '\0';

  result = concat (name, argv[1], NULL);

  free (name);
  return result;
}


// Source: gcc.c
// Lines 9944-9968
