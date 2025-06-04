__nlmsvc_proc_cancel(struct svc_rqst *rqstp, struct nlm_res *resp)
{
	struct nlm_args *argp = rqstp->rq_argp;
	struct nlm_host	*host;
	struct nlm_file	*file;
	struct net *net = SVC_NET(rqstp);

	dprintk("lockd: CANCEL        called\n");

	resp->cookie = argp->cookie;

	/* Don't accept requests during grace period */
	if (locks_in_grace(net)) {
		resp->status = nlm_lck_denied_grace_period;
		return rpc_success;
	}

	/* Obtain client and file */
	if ((resp->status = nlmsvc_retrieve_args(rqstp, argp, &host, &file)))
		return resp->status == nlm_drop_reply ? rpc_drop_reply :rpc_success;

	/* Try to cancel request. */
	resp->status = cast_status(nlmsvc_cancel_blocked(net, file, &argp->lock));

	dprintk("lockd: CANCEL        status %d\n", ntohl(resp->status));
	nlmsvc_release_lockowner(&argp->lock);
	nlmsvc_release_host(host);
	nlm_release_file(file);
	return rpc_success;
}


// Source: svcproc.c
// Lines 196-225
