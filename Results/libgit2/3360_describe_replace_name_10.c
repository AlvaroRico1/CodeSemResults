static int replace_name(
	git_tag **tag,
	git_repository *repo,
	struct commit_name *e,
	unsigned int prio,
	const git_oid *sha1)
{
	git_time_t e_time = 0, t_time = 0;

	if (!e || e->prio < prio)
		return 1;

	if (e->prio == 2 && prio == 2) {
		/* Multiple annotated tags point to the same commit.
		 * Select one to keep based upon their tagger date.
		 */
		git_tag *t = NULL;

		if (!e->tag) {
			if (git_tag_lookup(&t, repo, &e->sha1) < 0)
				return 1;
			e->tag = t;
		}

		if (git_tag_lookup(&t, repo, sha1) < 0)
			return 0;

		*tag = t;

		if (e->tag->tagger)
			e_time = e->tag->tagger->when.time;

		if (t->tagger)
			t_time = t->tagger->when.time;

		if (e_time < t_time)
			return 1;
	}


// Source: describe.c
// Lines 51-88
