void nfs_callback_down(int minorversion, struct net *net)
{
	struct nfs_callback_data *cb_info = &nfs_callback_info[minorversion];
	struct svc_serv *serv;

	mutex_lock(&nfs_callback_mutex);
	serv = cb_info->serv;
	nfs_callback_down_net(minorversion, serv, net);
	cb_info->users--;
	if (cb_info->users == 0) {
		svc_get(serv);
		serv->sv_ops->svo_setup(serv, NULL, 0);
		svc_destroy(serv);
		dprintk("nfs_callback_down: service destroyed\n");
		cb_info->serv = NULL;
	}
	mutex_unlock(&nfs_callback_mutex);
}


// Source: callback.c
// Lines 361-378
