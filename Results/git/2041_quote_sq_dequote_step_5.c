char *sq_dequote_step(char *arg, char **next)
{
	char *dst = arg;
	char *src = arg;
	char c;

	if (*src != '\'')
		return NULL;
	for (;;) {
		c = *++src;
		if (!c)
			return NULL;
		if (c != '\'') {
			*dst++ = c;
			continue;
		}
		/* We stepped out of sq */
		switch (*++src) {
		case '\0':
			*dst = 0;
			if (next)
				*next = NULL;
			return arg;
		case '\\':
			/*
			 * Allow backslashed characters outside of
			 * single-quotes only if they need escaping,
			 * and only if we resume the single-quoted part
			 * afterward.
			 */
			if (need_bs_quote(src[1]) && src[2] == '\'') {
				*dst++ = src[1];
				src += 2;
				continue;
			}
		/* Fallthrough */
		default:
			if (!next)
				return NULL;
			*dst = 0;
			*next = src;
			return arg;
		}
	}
}


// Source: quote.c
// Lines 119-163
