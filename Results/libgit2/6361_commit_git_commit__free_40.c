void git_commit__free(void *_commit)
{
	git_commit *commit = _commit;

	git_array_clear(commit->parent_ids);

	git_signature_free(commit->author);
	git_signature_free(commit->committer);

	git__free(commit->raw_header);
	git__free(commit->raw_message);
	git__free(commit->message_encoding);
	git__free(commit->summary);
	git__free(commit->body);

	git__free(commit);
}


// Source: commit.c
// Lines 26-42
