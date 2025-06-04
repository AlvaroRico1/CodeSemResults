int git_merge_base(git_oid *out, git_repository *repo, const git_oid *one, const git_oid *two)
{
	int error;
	git_revwalk *walk;
	git_commit_list *result;

	if ((error = merge_bases(&result, &walk, repo, one, two)) < 0)
		return error;

	git_oid_cpy(out, &result->item->oid);
	git_commit_list_free(&result);
	git_revwalk_free(walk);

	return 0;
}


// Source: merge.c
// Lines 266-280
