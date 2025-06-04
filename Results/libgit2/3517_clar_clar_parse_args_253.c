clar_parse_args(int argc, char **argv)
{
	int i;

	/* Verify options before execute */
	for (i = 1; i < argc; ++i) {
		char *argument = argv[i];

		if (argument[0] != '-' || argument[1] == '\0'
		    || strchr("sixvqQtlr", argument[1]) == NULL) {
			clar_usage(argv[0]);
		}
	}


// Source: clar.c
// Lines 383-395
