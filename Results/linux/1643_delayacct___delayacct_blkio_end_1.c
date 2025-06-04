void __delayacct_blkio_end(struct task_struct *p)
{
	struct task_delay_info *delays = p->delays;
	u64 *total;
	u32 *count;

	if (p->delays->flags & DELAYACCT_PF_SWAPIN) {
		total = &delays->swapin_delay;
		count = &delays->swapin_count;
	} else {
		total = &delays->blkio_delay;
		count = &delays->blkio_count;
	}

	delayacct_end(&delays->lock, &delays->blkio_start, total, count);
}


// Source: delayacct.c
// Lines 68-83
