nfs_page_group_lock(struct nfs_page *req)
{
	struct nfs_page *head = req->wb_head;

	WARN_ON_ONCE(head != head->wb_head);

	if (!test_and_set_bit(PG_HEADLOCK, &head->wb_flags))
		return 0;

	set_bit(PG_CONTENDED1, &head->wb_flags);
	smp_mb__after_atomic();
	return wait_on_bit_lock(&head->wb_flags, PG_HEADLOCK,
				TASK_UNINTERRUPTIBLE);
}


// Source: pagelist.c
// Lines 145-158
