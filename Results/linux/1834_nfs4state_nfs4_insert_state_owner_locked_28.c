nfs4_insert_state_owner_locked(struct nfs4_state_owner *new)
{
	struct nfs_server *server = new->so_server;
	struct rb_node **p = &server->state_owners.rb_node,
		       *parent = NULL;
	struct nfs4_state_owner *sp;
	int cmp;

	while (*p != NULL) {
		parent = *p;
		sp = rb_entry(parent, struct nfs4_state_owner, so_server_node);
		cmp = cred_fscmp(new->so_cred, sp->so_cred);

		if (cmp < 0)
			p = &parent->rb_left;
		else if (cmp > 0)
			p = &parent->rb_right;
		else {
			if (!list_empty(&sp->so_lru))
				list_del_init(&sp->so_lru);
			atomic_inc(&sp->so_count);
			return sp;
		}
	}
	rb_link_node(&new->so_server_node, parent, p);
	rb_insert_color(&new->so_server_node, &server->state_owners);
	return new;
}


// Source: nfs4state.c
// Lines 430-457
