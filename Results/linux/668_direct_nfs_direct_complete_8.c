static void nfs_direct_complete(struct nfs_direct_req *dreq)
{
	struct inode *inode = dreq->inode;

	inode_dio_end(inode);

	if (dreq->iocb) {
		long res = (long) dreq->error;
		if (dreq->count != 0) {
			res = (long) dreq->count;
			WARN_ON_ONCE(dreq->count < 0);
		}
		dreq->iocb->ki_complete(dreq->iocb, res, 0);
	}

	complete(&dreq->completion);

	nfs_direct_req_release(dreq);
}


// Source: direct.c
// Lines 379-397
