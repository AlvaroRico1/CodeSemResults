FUN(tok,line)(TYPE(Tokenizer) *tok, const TYPE(LineInfo) *line,
    int *argc, const Char ***argv, int *cursorc, int *cursoro)
{
	const Char *ptr;
	int cc, co;

	cc = co = -1;
	ptr = line->buffer;
	for (ptr = line->buffer; ;ptr++) {
		if (ptr >= line->lastchar)
			ptr = STR("");
		if (ptr == line->cursor) {
			cc = (int)tok->argc;
			co = (int)(tok->wptr - tok->wstart);
		}
		switch (*ptr) {
		case '\'':
			tok->flags |= TOK_KEEP;
			tok->flags &= ~TOK_EAT;
			switch (tok->quote) {
			case Q_none:
				tok->quote = Q_single;	/* Enter single quote
							 * mode */
				break;

			case Q_single:	/* Exit single quote mode */
				tok->quote = Q_none;
				break;

			case Q_one:	/* Quote this ' */
				tok->quote = Q_none;
				*tok->wptr++ = *ptr;
				break;

			case Q_double:	/* Stay in double quote mode */
				*tok->wptr++ = *ptr;
				break;

			case Q_doubleone:	/* Quote this ' */
				tok->quote = Q_double;
				*tok->wptr++ = *ptr;
				break;

			default:
				return -1;
			}
			break;

		case '"':
			tok->flags &= ~TOK_EAT;
			tok->flags |= TOK_KEEP;
			switch (tok->quote) {
			case Q_none:	/* Enter double quote mode */
				tok->quote = Q_double;
				break;

			case Q_double:	/* Exit double quote mode */
				tok->quote = Q_none;
				break;

			case Q_one:	/* Quote this " */
				tok->quote = Q_none;
				*tok->wptr++ = *ptr;
				break;

			case Q_single:	/* Stay in single quote mode */
				*tok->wptr++ = *ptr;
				break;

			case Q_doubleone:	/* Quote this " */
				tok->quote = Q_double;
				*tok->wptr++ = *ptr;
				break;

			default:
				return -1;
			}
			break;

		case '\\':
			tok->flags |= TOK_KEEP;
			tok->flags &= ~TOK_EAT;
			switch (tok->quote) {
			case Q_none:	/* Quote next character */
				tok->quote = Q_one;
				break;

			case Q_double:	/* Quote next character */
				tok->quote = Q_doubleone;
				break;

			case Q_one:	/* Quote this, restore state */
				*tok->wptr++ = *ptr;
				tok->quote = Q_none;
				break;

			case Q_single:	/* Stay in single quote mode */
				*tok->wptr++ = *ptr;
				break;

			case Q_doubleone:	/* Quote this \ */
				tok->quote = Q_double;
				*tok->wptr++ = *ptr;
				break;

			default:
				return -1;
			}
			break;

		case '\n':
			tok->flags &= ~TOK_EAT;
			switch (tok->quote) {
			case Q_none:
				goto tok_line_outok;

			case Q_single:
			case Q_double:
				*tok->wptr++ = *ptr;	/* Add the return */
				break;

			case Q_doubleone:   /* Back to double, eat the '\n' */
				tok->flags |= TOK_EAT;
				tok->quote = Q_double;
				break;

			case Q_one:	/* No quote, more eat the '\n' */
				tok->flags |= TOK_EAT;
				tok->quote = Q_none;
				break;

			default:
				return 0;
			}
			break;

		case '\0':
			switch (tok->quote) {
			case Q_none:
				/* Finish word and return */
				if (tok->flags & TOK_EAT) {
					tok->flags &= ~TOK_EAT;
					return 3;
				}
				goto tok_line_outok;

			case Q_single:
				return 1;

			case Q_double:
				return 2;

			case Q_doubleone:
				tok->quote = Q_double;
				*tok->wptr++ = *ptr;
				break;

			case Q_one:
				tok->quote = Q_none;
				*tok->wptr++ = *ptr;
				break;

			default:
				return -1;
			}
			break;

		default:
			tok->flags &= ~TOK_EAT;
			switch (tok->quote) {
			case Q_none:
				if (Strchr(tok->ifs, *ptr) != NULL)
					FUN(tok,finish)(tok);
				else
					*tok->wptr++ = *ptr;
				break;

			case Q_single:
			case Q_double:
				*tok->wptr++ = *ptr;
				break;


			case Q_doubleone:
				*tok->wptr++ = '\\';
				tok->quote = Q_double;
				*tok->wptr++ = *ptr;
				break;

			case Q_one:
				tok->quote = Q_none;
				*tok->wptr++ = *ptr;
				break;

			default:
				return -1;

			}
			break;
		}

		if (tok->wptr >= tok->wmax - 4) {
			size_t size = (size_t)(tok->wmax - tok->wspace + WINCR);
			Char *s = tok_realloc(tok->wspace,
			    size * sizeof(*s));
			if (s == NULL)
				return -1;

			if (s != tok->wspace) {
				size_t i;
				for (i = 0; i < tok->argc; i++) {
				    tok->argv[i] =
					(tok->argv[i] - tok->wspace) + s;
				}
				tok->wptr = (tok->wptr - tok->wspace) + s;
				tok->wstart = (tok->wstart - tok->wspace) + s;
				tok->wspace = s;
			}
			tok->wmax = s + size;
		}
		if (tok->argc >= tok->amax - 4) {
			Char **p;
			tok->amax += AINCR;
			p = tok_realloc(tok->argv, tok->amax * sizeof(*p));
			if (p == NULL) {
				tok->amax -= AINCR;
				return -1;
			}
			tok->argv = p;
		}
	}
 tok_line_outok:
	if (cc == -1 && co == -1) {
		cc = (int)tok->argc;
		co = (int)(tok->wptr - tok->wstart);
	}
	if (cursorc != NULL)
		*cursorc = cc;
	if (cursoro != NULL)
		*cursoro = co;
	FUN(tok,finish)(tok);
	*argv = (const Char **)tok->argv;
	*argc = (int)tok->argc;
	return 0;
}


// Source: tokenizer.c
// Lines 209-453
