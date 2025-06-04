static void nfs_commit_release(void *calldata)
{
	struct nfs_commit_data *data = calldata;

	data->completion_ops->completion(data);
	nfs_commitdata_release(calldata);
}


// Source: write.c
// Lines 1882-1888
