static void nfs_direct_write_reschedule_io(struct nfs_pgio_header *hdr)
{
	struct nfs_direct_req *dreq = hdr->dreq;

	spin_lock(&dreq->lock);
	if (dreq->error == 0) {
		dreq->flags = NFS_ODIRECT_RESCHED_WRITES;
		/* fake unstable write to let common nfs resend pages */
		hdr->verf.committed = NFS_UNSTABLE;
		hdr->good_bytes = hdr->args.count;
	}
	spin_unlock(&dreq->lock);
}


// Source: direct.c
// Lines 851-863
