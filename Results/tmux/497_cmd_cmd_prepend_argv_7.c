cmd_prepend_argv(int *argc, char ***argv, const char *arg)
{
	char	**new_argv;
	int	  i;

	new_argv = xreallocarray(NULL, (*argc) + 1, sizeof *new_argv);
	new_argv[0] = xstrdup(arg);
	for (i = 0; i < *argc; i++)
		new_argv[1 + i] = (*argv)[i];

	free(*argv);
	*argv = new_argv;
	(*argc)++;
}


// Source: cmd.c
// Lines 250-263
