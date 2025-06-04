void test_checkout_tree__filemode_preserved_in_index(void)
{
	git_oid executable_oid;
	git_commit *commit;
	git_checkout_options opts = GIT_CHECKOUT_OPTIONS_INIT;
	git_index *index;
	const git_index_entry *entry;

	opts.checkout_strategy = GIT_CHECKOUT_FORCE;

	cl_git_pass(git_repository_index(&index, g_repo));

	/* test a freshly added executable */
	cl_git_pass(git_oid_fromstr(&executable_oid, "afe4393b2b2a965f06acf2ca9658eaa01e0cd6b6"));
	cl_git_pass(git_commit_lookup(&commit, g_repo, &executable_oid));

	cl_git_pass(git_checkout_tree(g_repo, (const git_object *)commit, &opts));
	cl_assert(entry = git_index_get_bypath(index, "executable.txt", 0));
	cl_assert(GIT_PERMS_IS_EXEC(entry->mode));

	git_commit_free(commit);


	/* Now start with a commit which has a text file */
	cl_git_pass(git_oid_fromstr(&executable_oid, "cf80f8de9f1185bf3a05f993f6121880dd0cfbc9"));
	cl_git_pass(git_commit_lookup(&commit, g_repo, &executable_oid));

	cl_git_pass(git_checkout_tree(g_repo, (const git_object *)commit, &opts));
	cl_assert(entry = git_index_get_bypath(index, "a/b.txt", 0));
	cl_assert(!GIT_PERMS_IS_EXEC(entry->mode));

	git_commit_free(commit);


	/* And then check out to a commit which converts the text file to an executable */
	cl_git_pass(git_oid_fromstr(&executable_oid, "144344043ba4d4a405da03de3844aa829ae8be0e"));
	cl_git_pass(git_commit_lookup(&commit, g_repo, &executable_oid));

	cl_git_pass(git_checkout_tree(g_repo, (const git_object *)commit, &opts));
	cl_assert(entry = git_index_get_bypath(index, "a/b.txt", 0));
	cl_assert(GIT_PERMS_IS_EXEC(entry->mode));

	git_commit_free(commit);


	/* Finally, check out the text file again and check that the exec bit is cleared */
	cl_git_pass(git_oid_fromstr(&executable_oid, "cf80f8de9f1185bf3a05f993f6121880dd0cfbc9"));
	cl_git_pass(git_commit_lookup(&commit, g_repo, &executable_oid));

	cl_git_pass(git_checkout_tree(g_repo, (const git_object *)commit, &opts));
	cl_assert(entry = git_index_get_bypath(index, "a/b.txt", 0));
	cl_assert(!GIT_PERMS_IS_EXEC(entry->mode));

	git_commit_free(commit);


	git_index_free(index);
}


// Source: tree.c
// Lines 978-1035
