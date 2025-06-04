static int sort_ambiguous(const void *a, const void *b, void *ctx)
{
	struct repository *sort_ambiguous_repo = ctx;
	int a_type = oid_object_info(sort_ambiguous_repo, a, NULL);
	int b_type = oid_object_info(sort_ambiguous_repo, b, NULL);
	int a_type_sort;
	int b_type_sort;

	/*
	 * Sorts by hash within the same object type, just as
	 * oid_array_for_each_unique() would do.
	 */
	if (a_type == b_type)
		return oidcmp(a, b);

	/*
	 * Between object types show tags, then commits, and finally
	 * trees and blobs.
	 *
	 * The object_type enum is commit, tree, blob, tag, but we
	 * want tag, commit, tree blob. Cleverly (perhaps too
	 * cleverly) do that with modulus, since the enum assigns 1 to
	 * commit, so tag becomes 0.
	 */
	a_type_sort = a_type % 4;
	b_type_sort = b_type % 4;
	return a_type_sort > b_type_sort ? 1 : -1;
}


// Source: object-name.c
// Lines 399-426
