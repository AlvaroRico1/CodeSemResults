void test_refs_reflog_reflog__append_then_read(void)
{
	/* write a reflog for a given reference and ensure it can be read back */
	git_reference *ref;
	git_oid oid;
	git_signature *committer;
	git_reflog *reflog;

	/* Create a new branch pointing at the HEAD */
	git_oid_fromstr(&oid, current_master_tip);
	cl_git_pass(git_reference_create(&ref, g_repo, new_ref, &oid, 0, NULL));
	git_reference_free(ref);

	cl_git_pass(git_signature_now(&committer, "foo", "foo@bar"));

	cl_git_pass(git_reflog_read(&reflog, g_repo, new_ref));
	cl_git_pass(git_reflog_append(reflog, &oid, committer, NULL));
	cl_git_pass(git_reflog_append(reflog, &oid, committer, commit_msg "\n"));
	cl_git_pass(git_reflog_write(reflog));

	assert_appends(committer, &oid);

	git_reflog_free(reflog);
	git_signature_free(committer);
}


// Source: reflog.c
// Lines 74-98
