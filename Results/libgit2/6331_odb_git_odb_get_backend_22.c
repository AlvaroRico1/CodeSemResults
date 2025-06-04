int git_odb_get_backend(git_odb_backend **out, git_odb *odb, size_t pos)
{
	backend_internal *internal;
	int error;

	GIT_ASSERT_ARG(out);
	GIT_ASSERT_ARG(odb);


	if ((error = git_mutex_lock(&odb->lock)) < 0) {
		git_error_set(GIT_ERROR_ODB, "failed to acquire the odb lock");
		return error;
	}
	internal = git_vector_get(&odb->backends, pos);

	if (!internal || !internal->backend) {
		git_mutex_unlock(&odb->lock);

		git_error_set(GIT_ERROR_ODB, "no ODB backend loaded at index %" PRIuZ, pos);
		return GIT_ENOTFOUND;
	}
	*out = internal->backend;
	git_mutex_unlock(&odb->lock);

	return 0;
}


// Source: odb.c
// Lines 543-568
