status_prompt_complete_prefix(char **list, u_int size)
{
	char	 *out;
	u_int	  i;
	size_t	  j;

	if (list == NULL || size == 0)
		return (NULL);
	out = xstrdup(list[0]);
	for (i = 1; i < size; i++) {
		j = strlen(list[i]);
		if (j > strlen(out))
			j = strlen(out);
		for (; j > 0; j--) {
			if (out[j - 1] != list[i][j - 1])
				out[j - 1] = '\0';
		}
	}
	return (out);
}


// Source: status.c
// Lines 1631-1650
