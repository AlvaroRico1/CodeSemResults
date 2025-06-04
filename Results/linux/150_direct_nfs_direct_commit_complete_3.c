static void nfs_direct_commit_complete(struct nfs_commit_data *data)
{
	struct nfs_direct_req *dreq = data->dreq;
	struct nfs_commit_info cinfo;
	struct nfs_page *req;
	int status = data->task.tk_status;

	nfs_init_cinfo_from_dreq(&cinfo, dreq);
	if (status < 0 || nfs_direct_cmp_commit_data_verf(dreq, data))
		dreq->flags = NFS_ODIRECT_RESCHED_WRITES;

	while (!list_empty(&data->pages)) {
		req = nfs_list_entry(data->pages.next);
		nfs_list_remove_request(req);
		if (dreq->flags == NFS_ODIRECT_RESCHED_WRITES) {
			/*
			 * Despite the reboot, the write was successful,
			 * so reset wb_nio.
			 */
			req->wb_nio = 0;
			/* Note the rewrite will go through mds */
			nfs_mark_request_commit(req, NULL, &cinfo, 0);
		} else
			nfs_release_request(req);
		nfs_unlock_and_release_request(req);
	}

	if (atomic_dec_and_test(&cinfo.mds->rpcs_out))
		nfs_direct_write_complete(dreq);
}


// Source: direct.c
// Lines 700-729
