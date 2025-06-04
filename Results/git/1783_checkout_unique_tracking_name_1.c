const char *unique_tracking_name(const char *name, struct object_id *oid,
				 int *dwim_remotes_matched)
{
	struct tracking_name_data cb_data = TRACKING_NAME_DATA_INIT;
	const char *default_remote = NULL;
	if (!git_config_get_string_tmp("checkout.defaultremote", &default_remote))
		cb_data.default_remote = default_remote;
	cb_data.src_ref = xstrfmt("refs/heads/%s", name);
	cb_data.dst_oid = oid;
	for_each_remote(check_tracking_name, &cb_data);
	if (dwim_remotes_matched)
		*dwim_remotes_matched = cb_data.num_matches;
	free(cb_data.src_ref);
	if (cb_data.num_matches == 1) {
		free(cb_data.default_dst_ref);
		free(cb_data.default_dst_oid);
		return cb_data.dst_ref;
	}
	free(cb_data.dst_ref);
	if (cb_data.default_dst_ref) {
		oidcpy(oid, cb_data.default_dst_oid);
		free(cb_data.default_dst_oid);
		return cb_data.default_dst_ref;
	}
	return NULL;
}


// Source: checkout.c
// Lines 45-70
