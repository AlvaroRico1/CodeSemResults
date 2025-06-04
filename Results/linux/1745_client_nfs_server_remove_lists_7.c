void nfs_server_remove_lists(struct nfs_server *server)
{
	struct nfs_client *clp = server->nfs_client;
	struct nfs_net *nn;

	if (clp == NULL)
		return;
	nn = net_generic(clp->cl_net, nfs_net_id);
	spin_lock(&nn->nfs_client_lock);
	list_del_rcu(&server->client_link);
	if (list_empty(&clp->cl_superblocks))
		set_bit(NFS_CS_STOP_RENEW, &clp->cl_res_state);
	list_del(&server->master_link);
	spin_unlock(&nn->nfs_client_lock);

	synchronize_rcu();
}


// Source: client.c
// Lines 856-872
