static void diff_parsed_free(git_diff *d)
{
	git_diff_parsed *diff = (git_diff_parsed *)d;
	git_patch *patch;
	size_t i;

	git_vector_foreach(&diff->patches, i, patch)
		git_patch_free(patch);

	git_vector_free(&diff->patches);

	git_vector_free(&diff->base.deltas);
	git_pool_clear(&diff->base.pool);

	git__memzero(diff, sizeof(*diff));
	git__free(diff);
}


// Source: diff_parse.c
// Lines 14-30
