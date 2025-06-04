void test_status_worktree__conflict_with_diff3(void)
{
	git_repository *repo = cl_git_sandbox_init("status");
	git_index *index;
	unsigned int status;
	git_index_entry ancestor_entry, our_entry, their_entry;

	memset(&ancestor_entry, 0x0, sizeof(git_index_entry));
	memset(&our_entry, 0x0, sizeof(git_index_entry));
	memset(&their_entry, 0x0, sizeof(git_index_entry));

	ancestor_entry.path = "modified_file";
	ancestor_entry.mode = 0100644;
	git_oid_fromstr(&ancestor_entry.id,
		"452e4244b5d083ddf0460acf1ecc74db9dcfa11a");

	our_entry.path = "modified_file";
	our_entry.mode = 0100644;
	git_oid_fromstr(&our_entry.id,
		"452e4244b5d083ddf0460acf1ecc74db9dcfa11a");

	their_entry.path = "modified_file";
	their_entry.mode = 0100644;
	git_oid_fromstr(&their_entry.id,
		"452e4244b5d083ddf0460acf1ecc74db9dcfa11a");

	cl_git_pass(git_status_file(&status, repo, "modified_file"));
	cl_assert_equal_i(GIT_STATUS_WT_MODIFIED, status);

	cl_git_pass(git_repository_index(&index, repo));
	cl_git_pass(git_index_remove(index, "modified_file", 0));
	cl_git_pass(git_index_conflict_add(
		index, &ancestor_entry, &our_entry, &their_entry));
	cl_git_pass(git_index_write(index));
	git_index_free(index);

	cl_git_pass(git_status_file(&status, repo, "modified_file"));

	cl_assert_equal_i(GIT_STATUS_CONFLICTED, status);
}


// Source: worktree.c
// Lines 505-544
