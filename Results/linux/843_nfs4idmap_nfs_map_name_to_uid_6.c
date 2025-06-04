int nfs_map_name_to_uid(const struct nfs_server *server, const char *name, size_t namelen, kuid_t *uid)
{
	struct idmap *idmap = server->nfs_client->cl_idmap;
	__u32 id = -1;
	int ret = 0;

	if (!nfs_map_string_to_numeric(name, namelen, &id))
		ret = nfs_idmap_lookup_id(name, namelen, "uid", &id, idmap);
	if (ret == 0) {
		*uid = make_kuid(idmap_userns(idmap), id);
		if (!uid_valid(*uid))
			ret = -ERANGE;
	}
	trace_nfs4_map_name_to_uid(name, namelen, id, ret);
	return ret;
}


// Source: nfs4idmap.c
// Lines 742-757
