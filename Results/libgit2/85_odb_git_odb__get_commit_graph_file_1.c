int git_odb__get_commit_graph_file(git_commit_graph_file **out, git_odb *db)
{
	int error = 0;
	git_commit_graph_file *result = NULL;

	if ((error = git_mutex_lock(&db->lock)) < 0) {
		git_error_set(GIT_ERROR_ODB, "failed to acquire the db lock");
		return error;
	}
	if (!db->cgraph) {
		error = GIT_ENOTFOUND;
		goto done;
	}
	error = git_commit_graph_get_file(&result, db->cgraph);
	if (error)
		goto done;
	*out = result;

done:
	git_mutex_unlock(&db->lock);
	return error;
}


// Source: odb.c
// Lines 814-835
