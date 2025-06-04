void merge_switch_to_result(struct merge_options *opt,
			    struct tree *head,
			    struct merge_result *result,
			    int update_worktree_and_index,
			    int display_update_msgs)
{
	assert(opt->priv == NULL);
	if (result->clean >= 0 && update_worktree_and_index) {
		const char *filename;
		FILE *fp;

		trace2_region_enter("merge", "checkout", opt->repo);
		if (checkout(opt, head, result->tree)) {
			/* failure to function */
			result->clean = -1;
			return;
		}
		trace2_region_leave("merge", "checkout", opt->repo);

		trace2_region_enter("merge", "record_conflicted", opt->repo);
		opt->priv = result->priv;
		if (record_conflicted_index_entries(opt)) {
			/* failure to function */
			opt->priv = NULL;
			result->clean = -1;
			return;
		}
		opt->priv = NULL;
		trace2_region_leave("merge", "record_conflicted", opt->repo);

		trace2_region_enter("merge", "write_auto_merge", opt->repo);
		filename = git_path_auto_merge(opt->repo);
		fp = xfopen(filename, "w");
		fprintf(fp, "%s\n", oid_to_hex(&result->tree->object.oid));
		fclose(fp);
		trace2_region_leave("merge", "write_auto_merge", opt->repo);
	}

	if (display_update_msgs) {
		struct merge_options_internal *opti = result->priv;
		struct hashmap_iter iter;
		struct strmap_entry *e;
		struct string_list olist = STRING_LIST_INIT_NODUP;
		int i;

		trace2_region_enter("merge", "display messages", opt->repo);

		/* Hack to pre-allocate olist to the desired size */
		ALLOC_GROW(olist.items, strmap_get_size(&opti->output),
			   olist.alloc);

		/* Put every entry from output into olist, then sort */
		strmap_for_each_entry(&opti->output, &iter, e) {
			string_list_append(&olist, e->key)->util = e->value;
		}
		string_list_sort(&olist);

		/* Iterate over the items, printing them */
		for (i = 0; i < olist.nr; ++i) {
			struct strbuf *sb = olist.items[i].util;

			printf("%s", sb->buf);
		}
		string_list_clear(&olist, 0);

		/* Also include needed rename limit adjustment now */
		diff_warn_rename_limit("merge.renamelimit",
				       opti->renames.needed_limit, 0);

		trace2_region_leave("merge", "display messages", opt->repo);
	}


// Source: merge-ort.c
// Lines 4210-4280
