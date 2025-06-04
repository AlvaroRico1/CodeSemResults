int git_submodule_foreach(
	git_repository *repo,
	git_submodule_cb callback,
	void *payload)
{
	git_vector snapshot = GIT_VECTOR_INIT;
	git_strmap *submodules;
	git_submodule *sm;
	int error;
	size_t i;

	if (repo->is_bare) {
		git_error_set(GIT_ERROR_SUBMODULE, "cannot get submodules without a working tree");
		return -1;
	}

	if ((error = git_strmap_new(&submodules)) < 0)
		return error;

	if ((error = git_submodule__map(repo, submodules)) < 0)
		goto done;

	if (!(error = git_vector_init(
			&snapshot, git_strmap_size(submodules), submodule_cmp))) {

		git_strmap_foreach_value(submodules, sm, {
			if ((error = git_vector_insert(&snapshot, sm)) < 0)
				break;
			GIT_REFCOUNT_INC(sm);
		});
	}

	if (error < 0)
		goto done;

	git_vector_uniq(&snapshot, submodule_free_dup);

	git_vector_foreach(&snapshot, i, sm) {
		if ((error = callback(sm, sm->name, payload)) != 0) {
			git_error_set_after_callback(error);
			break;
		}
	}

done:
	git_vector_foreach(&snapshot, i, sm)
		git_submodule_free(sm);
	git_vector_free(&snapshot);

	git_strmap_foreach_value(submodules, sm, {
		git_submodule_free(sm);
	});
	git_strmap_free(submodules);

	return error;
}


// Source: submodule.c
// Lines 624-679
