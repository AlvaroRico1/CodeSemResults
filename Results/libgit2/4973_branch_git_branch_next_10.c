int git_branch_next(git_reference **out, git_branch_t *out_type, git_branch_iterator *_iter)
{
	branch_iter *iter = (branch_iter *) _iter;
	git_reference *ref;
	int error;

	while ((error = git_reference_next(&ref, iter->iter)) == 0) {
		if ((iter->flags & GIT_BRANCH_LOCAL) &&
		    !git__prefixcmp(ref->name, GIT_REFS_HEADS_DIR)) {
			*out = ref;
			*out_type = GIT_BRANCH_LOCAL;

			return 0;
		} else  if ((iter->flags & GIT_BRANCH_REMOTE) &&
			    !git__prefixcmp(ref->name, GIT_REFS_REMOTES_DIR)) {
			*out = ref;
			*out_type = GIT_BRANCH_REMOTE;

			return 0;
		} else {
			git_reference_free(ref);
		}
	}

	return error;
}


// Source: branch.c
// Lines 227-252
