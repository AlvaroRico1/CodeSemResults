regsub_expand(char **buf, size_t *len, const char *with, const char *text,
    regmatch_t *m, u_int n)
{
	const char	*cp;
	u_int		 i;

	for (cp = with; *cp != '\0'; cp++) {
		if (*cp == '\\') {
			cp++;
			if (*cp >= '0' && *cp <= '9') {
				i = *cp - '0';
				if (i < n && m[i].rm_so != m[i].rm_eo) {
					regsub_copy(buf, len, text, m[i].rm_so,
					    m[i].rm_eo);
					continue;
				}
			}
		}
		*buf = xrealloc(*buf, (*len) + 2);
		(*buf)[(*len)++] = *cp;
	}
}


// Source: regsub.c
// Lines 37-58
