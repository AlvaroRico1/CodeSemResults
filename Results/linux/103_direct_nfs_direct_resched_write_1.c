static void nfs_direct_resched_write(struct nfs_commit_info *cinfo,
		struct nfs_page *req)
{
	struct nfs_direct_req *dreq = cinfo->dreq;

	spin_lock(&dreq->lock);
	dreq->flags = NFS_ODIRECT_RESCHED_WRITES;
	spin_unlock(&dreq->lock);
	nfs_mark_request_commit(req, NULL, cinfo, 0);
}


// Source: direct.c
// Lines 731-740
