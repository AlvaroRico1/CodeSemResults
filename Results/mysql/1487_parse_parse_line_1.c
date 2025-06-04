parse_line(EditLine *el, const wchar_t *line)
{
	const wchar_t **argv = NULL;
	int argc = 0;
	TokenizerW *tok;

	tok = tok_winit(NULL);
	tok_wstr(tok, line, &argc, &argv);
	argc = el_wparse(el, argc, argv);
	tok_wend(tok);
	return argc;
}


// Source: parse.c
// Lines 82-93
