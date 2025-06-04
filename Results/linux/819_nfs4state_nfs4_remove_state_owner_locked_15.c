nfs4_remove_state_owner_locked(struct nfs4_state_owner *sp)
{
	struct nfs_server *server = sp->so_server;

	if (!RB_EMPTY_NODE(&sp->so_server_node))
		rb_erase(&sp->so_server_node, &server->state_owners);
}


// Source: nfs4state.c
// Lines 460-466
