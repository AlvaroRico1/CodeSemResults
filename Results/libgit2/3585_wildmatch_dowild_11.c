static int dowild(const uchar *p, const uchar *text, unsigned int flags)
{
	uchar p_ch;
	const uchar *pattern = p;

	for ( ; (p_ch = *p) != '\0'; text++, p++) {
		int matched, match_slash, negated;
		uchar t_ch, prev_ch;
		if ((t_ch = *text) == '\0' && p_ch != '*')
			return WM_ABORT_ALL;
		if ((flags & WM_CASEFOLD) && ISUPPER(t_ch))
			t_ch = tolower(t_ch);
		if ((flags & WM_CASEFOLD) && ISUPPER(p_ch))
			p_ch = tolower(p_ch);
		switch (p_ch) {
		case '\\':
			/* Literal match with following character.  Note that the test
			 * in "default" handles the p[1] == '\0' failure case. */
			p_ch = *++p;
			/* FALLTHROUGH */
		default:
			if (t_ch != p_ch)
				return WM_NOMATCH;
			continue;
		case '?':
			/* Match anything but '/'. */
			if ((flags & WM_PATHNAME) && t_ch == '/')
				return WM_NOMATCH;
			continue;
		case '*':
			if (*++p == '*') {
				const uchar *prev_p = p - 2;
				while (*++p == '*') {}
				if (!(flags & WM_PATHNAME))
					/* without WM_PATHNAME, '*' == '**' */
					match_slash = 1;
				else if ((prev_p < pattern || *prev_p == '/') &&
				    (*p == '\0' || *p == '/' ||
				     (p[0] == '\\' && p[1] == '/'))) {
					/*
					 * Assuming we already match 'foo/' and are at
					 * <star star slash>, just assume it matches
					 * nothing and go ahead match the rest of the
					 * pattern with the remaining string. This
					 * helps make foo/<*><*>/bar (<> because
					 * otherwise it breaks C comment syntax) match
					 * both foo/bar and foo/a/bar.
					 */
					if (p[0] == '/' &&
					    dowild(p + 1, text, flags) == WM_MATCH)
						return WM_MATCH;
					match_slash = 1;
				} else /* WM_PATHNAME is set */
					match_slash = 0;
			} else


// Source: wildmatch.c
// Lines 97-151
