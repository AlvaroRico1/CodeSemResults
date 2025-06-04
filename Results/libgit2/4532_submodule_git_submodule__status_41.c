int git_submodule__status(
	unsigned int *out_status,
	git_oid *out_head_id,
	git_oid *out_index_id,
	git_oid *out_wd_id,
	git_submodule *sm,
	git_submodule_ignore_t ign)
{
	unsigned int status;
	git_repository *smrepo = NULL;

	if (ign == GIT_SUBMODULE_IGNORE_UNSPECIFIED)
		ign = sm->ignore;

	/* only return location info if ignore == all */
	if (ign == GIT_SUBMODULE_IGNORE_ALL) {
		*out_status = (sm->flags & GIT_SUBMODULE_STATUS__IN_FLAGS);
		return 0;
	}

	/* If the user has requested caching submodule state, performing these
	 * expensive operations (especially `submodule_update_head`, which is
	 * bottlenecked on `git_repository_head_tree`) eliminates much of the
	 * advantage.  We will, therefore, interpret the request for caching to
	 * apply here to and skip them.
	 */

	if (sm->repo->submodule_cache == NULL) {
		/* refresh the index OID */
		if (submodule_update_index(sm) < 0)
			return -1;

		/* refresh the HEAD OID */
		if (submodule_update_head(sm) < 0)
			return -1;
	}

	/* for ignore == dirty, don't scan the working directory */
	if (ign == GIT_SUBMODULE_IGNORE_DIRTY) {
		/* git_submodule_open_bare will load WD OID data */
		if (git_submodule_open_bare(&smrepo, sm) < 0)
			git_error_clear();
		else
			git_repository_free(smrepo);
		smrepo = NULL;
	} else if (git_submodule_open(&smrepo, sm) < 0) {
		git_error_clear();
		smrepo = NULL;
	}

	status = GIT_SUBMODULE_STATUS__CLEAR_INTERNAL(sm->flags);

	submodule_get_index_status(&status, sm);
	submodule_get_wd_status(&status, sm, smrepo, ign);

	git_repository_free(smrepo);

	*out_status = status;

	submodule_copy_oid_maybe(out_head_id, &sm->head_oid,
		(sm->flags & GIT_SUBMODULE_STATUS__HEAD_OID_VALID) != 0);
	submodule_copy_oid_maybe(out_index_id, &sm->index_oid,
		(sm->flags & GIT_SUBMODULE_STATUS__INDEX_OID_VALID) != 0);
	submodule_copy_oid_maybe(out_wd_id, &sm->wd_oid,
		(sm->flags & GIT_SUBMODULE_STATUS__WD_OID_VALID) != 0);

	return 0;
}


// Source: submodule.c
// Lines 1720-1787
