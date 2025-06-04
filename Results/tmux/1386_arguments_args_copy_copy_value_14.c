args_copy_copy_value(struct args_value *to, struct args_value *from, int argc,
    char **argv)
{
	char	*s, *expanded;
	int	 i;

	to->type = from->type;
	switch (from->type) {
	case ARGS_NONE:
		break;
	case ARGS_STRING:
		expanded = xstrdup(from->string);
		for (i = 0; i < argc; i++) {
			s = cmd_template_replace(expanded, argv[i], i + 1);
			free(expanded);
			expanded = s;
		}
		to->string = expanded;
		break;
	case ARGS_COMMANDS:
		to->cmdlist = cmd_list_copy(from->cmdlist, argc, argv);
		break;
	}
}


// Source: arguments.c
// Lines 275-298
