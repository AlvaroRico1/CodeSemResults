void test_revert_bare__orphan(void)
{
	git_commit *head_commit, *revert_commit;
	git_oid head_oid, revert_oid;
	git_index *index;

	struct merge_index_entry merge_index_entries[] = {
		{ 0100644, "296a6d3be1dff05c5d1f631d2459389fa7b619eb", 0, "file-mainline.txt" },
	};

	git_oid_fromstr(&head_oid, "39467716290f6df775a91cdb9a4eb39295018145");
	cl_git_pass(git_commit_lookup(&head_commit, repo, &head_oid));

	git_oid_fromstr(&revert_oid, "ebb03002cee5d66c7732dd06241119fe72ab96a5");
	cl_git_pass(git_commit_lookup(&revert_commit, repo, &revert_oid));

	cl_git_pass(git_revert_commit(&index, repo, revert_commit, head_commit, 0, NULL));
	cl_assert(merge_test_index(index, merge_index_entries, 1));

	git_commit_free(revert_commit);
	git_commit_free(head_commit);
	git_index_free(index);
}


// Source: bare.c
// Lines 84-106
