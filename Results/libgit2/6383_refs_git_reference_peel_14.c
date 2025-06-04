int git_reference_peel(
	git_object **peeled,
	const git_reference *ref,
	git_object_t target_type)
{
	const git_reference *resolved = NULL;
	git_reference *allocated = NULL;
	git_object *target = NULL;
	int error;

	GIT_ASSERT_ARG(ref);

	if (ref->type == GIT_REFERENCE_DIRECT) {
		resolved = ref;
	} else {
		if ((error = git_reference_resolve(&allocated, ref)) < 0)
			return peel_error(error, ref, "Cannot resolve reference");

		resolved = allocated;
	}

	/*
	 * If we try to peel an object to a tag, we cannot use
	 * the fully peeled object, as that will always resolve
	 * to a commit. So we only want to use the peeled value
	 * if it is not zero and the target is not a tag.
	 */
	if (target_type != GIT_OBJECT_TAG && !git_oid_is_zero(&resolved->peel)) {
		error = git_object_lookup(&target,
			git_reference_owner(ref), &resolved->peel, GIT_OBJECT_ANY);
	} else {
		error = git_object_lookup(&target,
			git_reference_owner(ref), &resolved->target.oid, GIT_OBJECT_ANY);
	}

	if (error < 0) {
		peel_error(error, ref, "Cannot retrieve reference target");
		goto cleanup;
	}

	if (target_type == GIT_OBJECT_ANY && git_object_type(target) != GIT_OBJECT_TAG)
		error = git_object_dup(peeled, target);
	else
		error = git_object_peel(peeled, target, target_type);

cleanup:
	git_object_free(target);
	git_reference_free(allocated);

	return error;
}


// Source: refs.c
// Lines 1257-1307
