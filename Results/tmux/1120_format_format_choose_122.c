format_choose(struct format_expand_state *es, const char *s, char **left,
    char **right, int expand)
{
	const char	*cp;
	char		*left0, *right0;

	cp = format_skip(s, ",");
	if (cp == NULL)
		return (-1);
	left0 = xstrndup(s, cp - s);
	right0 = xstrdup(cp + 1);

	if (expand) {
		*left = format_expand1(es, left0);
		free(left0);
		*right = format_expand1(es, right0);
		free(right0);
	} else {
		*left = left0;
		*right = right0;
	}
	return (0);
}


// Source: format.c
// Lines 3477-3499
