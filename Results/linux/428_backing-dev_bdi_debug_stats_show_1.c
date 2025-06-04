static int bdi_debug_stats_show(struct seq_file *m, void *v)
{
	struct backing_dev_info *bdi = m->private;
	struct bdi_writeback *wb = &bdi->wb;
	unsigned long background_thresh;
	unsigned long dirty_thresh;
	unsigned long wb_thresh;
	unsigned long nr_dirty, nr_io, nr_more_io, nr_dirty_time;
	struct inode *inode;

	nr_dirty = nr_io = nr_more_io = nr_dirty_time = 0;
	spin_lock(&wb->list_lock);
	list_for_each_entry(inode, &wb->b_dirty, i_io_list)
		nr_dirty++;
	list_for_each_entry(inode, &wb->b_io, i_io_list)
		nr_io++;
	list_for_each_entry(inode, &wb->b_more_io, i_io_list)
		nr_more_io++;
	list_for_each_entry(inode, &wb->b_dirty_time, i_io_list)
		if (inode->i_state & I_DIRTY_TIME)
			nr_dirty_time++;
	spin_unlock(&wb->list_lock);

	global_dirty_limits(&background_thresh, &dirty_thresh);
	wb_thresh = wb_calc_thresh(wb, dirty_thresh);

#define K(x) ((x) << (PAGE_SHIFT - 10))
	seq_printf(m,
		   "BdiWriteback:       %10lu kB\n"
		   "BdiReclaimable:     %10lu kB\n"
		   "BdiDirtyThresh:     %10lu kB\n"
		   "DirtyThresh:        %10lu kB\n"
		   "BackgroundThresh:   %10lu kB\n"
		   "BdiDirtied:         %10lu kB\n"
		   "BdiWritten:         %10lu kB\n"
		   "BdiWriteBandwidth:  %10lu kBps\n"
		   "b_dirty:            %10lu\n"
		   "b_io:               %10lu\n"
		   "b_more_io:          %10lu\n"
		   "b_dirty_time:       %10lu\n"
		   "bdi_list:           %10u\n"
		   "state:              %10lx\n",
		   (unsigned long) K(wb_stat(wb, WB_WRITEBACK)),
		   (unsigned long) K(wb_stat(wb, WB_RECLAIMABLE)),
		   K(wb_thresh),
		   K(dirty_thresh),
		   K(background_thresh),
		   (unsigned long) K(wb_stat(wb, WB_DIRTIED)),
		   (unsigned long) K(wb_stat(wb, WB_WRITTEN)),
		   (unsigned long) K(wb->write_bandwidth),
		   nr_dirty,
		   nr_io,
		   nr_more_io,
		   nr_dirty_time,
		   !list_empty(&bdi->bdi_list), bdi->wb.state);
#undef K

	return 0;
}


// Source: backing-dev.c
// Lines 45-103
