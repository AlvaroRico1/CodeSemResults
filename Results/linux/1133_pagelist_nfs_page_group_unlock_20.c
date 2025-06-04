nfs_page_group_unlock(struct nfs_page *req)
{
	struct nfs_page *head = req->wb_head;

	WARN_ON_ONCE(head != head->wb_head);

	smp_mb__before_atomic();
	clear_bit(PG_HEADLOCK, &head->wb_flags);
	smp_mb__after_atomic();
	if (!test_bit(PG_CONTENDED1, &head->wb_flags))
		return;
	wake_up_bit(&head->wb_flags, PG_HEADLOCK);
}


// Source: pagelist.c
// Lines 165-177
