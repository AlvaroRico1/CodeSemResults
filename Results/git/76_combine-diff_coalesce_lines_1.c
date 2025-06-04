static struct lline *coalesce_lines(struct lline *base, int *lenbase,
				    struct lline *newline, int lennew,
				    unsigned long parent, long flags)
{
	int **lcs;
	enum coalesce_direction **directions;
	struct lline *baseend, *newend = NULL;
	int i, j, origbaselen = *lenbase;

	if (newline == NULL)
		return base;

	if (base == NULL) {
		*lenbase = lennew;
		return newline;
	}

	/*
	 * Coalesce new lines into base by finding the LCS
	 * - Create the table to run dynamic programming
	 * - Compute the LCS
	 * - Then reverse read the direction structure:
	 *   - If we have MATCH, assign parent to base flag, and consume
	 *   both baseend and newend
	 *   - Else if we have BASE, consume baseend
	 *   - Else if we have NEW, insert newend lline into base and
	 *   consume newend
	 */
	CALLOC_ARRAY(lcs, st_add(origbaselen, 1));
	CALLOC_ARRAY(directions, st_add(origbaselen, 1));
	for (i = 0; i < origbaselen + 1; i++) {
		CALLOC_ARRAY(lcs[i], st_add(lennew, 1));
		CALLOC_ARRAY(directions[i], st_add(lennew, 1));
		directions[i][0] = BASE;
	}
	for (j = 1; j < lennew + 1; j++)
		directions[0][j] = NEW;

	for (i = 1, baseend = base; i < origbaselen + 1; i++) {
		for (j = 1, newend = newline; j < lennew + 1; j++) {
			if (match_string_spaces(baseend->line, baseend->len,
						newend->line, newend->len, flags)) {
				lcs[i][j] = lcs[i - 1][j - 1] + 1;
				directions[i][j] = MATCH;
			} else if (lcs[i][j - 1] >= lcs[i - 1][j]) {
				lcs[i][j] = lcs[i][j - 1];
				directions[i][j] = NEW;
			} else {
				lcs[i][j] = lcs[i - 1][j];
				directions[i][j] = BASE;
			}
			if (newend->next)
				newend = newend->next;
		}
		if (baseend->next)
			baseend = baseend->next;
	}

	for (i = 0; i < origbaselen + 1; i++)
		free(lcs[i]);
	free(lcs);

	/* At this point, baseend and newend point to the end of each lists */
	i--;
	j--;
	while (i != 0 || j != 0) {
		if (directions[i][j] == MATCH) {
			baseend->parent_map |= 1<<parent;
			baseend = baseend->prev;
			newend = newend->prev;
			i--;
			j--;
		} else if (directions[i][j] == NEW) {
			struct lline *lline;

			lline = newend;
			/* Remove lline from new list and update newend */
			if (lline->prev)
				lline->prev->next = lline->next;
			else
				newline = lline->next;
			if (lline->next)
				lline->next->prev = lline->prev;

			newend = lline->prev;
			j--;

			/* Add lline to base list */
			if (baseend) {
				lline->next = baseend->next;
				lline->prev = baseend;
				if (lline->prev)
					lline->prev->next = lline;
			}
			else {
				lline->next = base;
				base = lline;
			}
			(*lenbase)++;

			if (lline->next)
				lline->next->prev = lline;

		} else {
			baseend = baseend->prev;
			i--;
		}
	}

	newend = newline;
	while (newend) {
		struct lline *lline = newend;
		newend = newend->next;
		free(lline);
	}

	for (i = 0; i < origbaselen + 1; i++)
		free(directions[i]);
	free(directions);

	return base;
}


// Source: combine-diff.c
// Lines 189-310
