static void fill_index_wth_head_entries(git_repository *repo, git_index *index)
{
	git_oid oid;
	git_commit *commit;
	git_tree *tree;

	cl_git_pass(git_reference_name_to_id(&oid, repo, "HEAD"));
	cl_git_pass(git_commit_lookup(&commit, repo, &oid));
	cl_git_pass(git_commit_tree(&tree, commit));

	cl_git_pass(git_index_read_tree(index, tree));
	cl_git_pass(git_index_write(index));

	git_tree_free(tree);
	git_commit_free(commit);
}


// Source: worktree_init.c
// Lines 81-96
