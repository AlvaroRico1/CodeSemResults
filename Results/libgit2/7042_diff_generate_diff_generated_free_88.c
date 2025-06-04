static void diff_generated_free(git_diff *d)
{
	git_diff_generated *diff = (git_diff_generated *)d;

	git_attr_session__free(&diff->base.attrsession);
	git_vector_free_deep(&diff->base.deltas);

	git_pathspec__vfree(&diff->pathspec);
	git_pool_clear(&diff->base.pool);

	git__memzero(diff, sizeof(*diff));
	git__free(diff);
}


// Source: diff_generate.c
// Lines 413-425
