int git_blame__like_git(git_blame *blame, uint32_t opt)
{
	int error = 0;

	while (true) {
		git_blame__entry *ent;
		git_blame__origin *suspect = NULL;

		/* Find a suspect to break down */
		for (ent = blame->ent; !suspect && ent; ent = ent->next)
			if (!ent->guilty)
				suspect = ent->suspect;
		if (!suspect)
			break;

		/* We'll use this suspect later in the loop, so hold on to it for now. */
		origin_incref(suspect);

		if ((error = pass_blame(blame, suspect, opt)) < 0)
			break;

		/* Take responsibility for the remaining entries */
		for (ent = blame->ent; ent; ent = ent->next) {
			if (same_suspect(ent->suspect, suspect)) {
				ent->guilty = true;
				ent->is_boundary = !git_oid_cmp(
						git_commit_id(suspect->commit),
						&blame->options.oldest_commit);
			}
		}
		origin_decref(suspect);
	}


// Source: blame_git.c
// Lines 641-672
