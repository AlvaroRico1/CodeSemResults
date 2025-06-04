static void nfs4_setclientid_done(struct rpc_task *task, void *calldata)
{
	struct nfs4_setclientid *sc = calldata;

	if (task->tk_status == 0)
		sc->sc_cred = get_rpccred(task->tk_rqstp->rq_cred);
}


// Source: nfs4proc.c
// Lines 5983-5989
