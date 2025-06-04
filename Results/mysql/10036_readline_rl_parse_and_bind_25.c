rl_parse_and_bind(const char *line)
{
	const char **argv;
	int argc;
	Tokenizer *tok;

	tok = tok_init(NULL);
	tok_str(tok, line, &argc, &argv);
	argc = el_parse(e, argc, argv);
	tok_end(tok);
	return argc ? 1 : 0;
}


// Source: readline.c
// Lines 2147-2158
