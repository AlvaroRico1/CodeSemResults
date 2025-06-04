static void parse_and_save_result(const char *buffer, int len,
				  struct pc_worker *worker)
{
	struct pc_item_result *res;
	struct parallel_checkout_item *pc_item;
	struct stat *st = NULL;

	if (len < PC_ITEM_RESULT_BASE_SIZE)
		BUG("too short result from checkout worker (got %dB, exp >=%dB)",
		    len, (int)PC_ITEM_RESULT_BASE_SIZE);

	res = (struct pc_item_result *)buffer;

	/*
	 * Worker should send either the full result struct on success, or
	 * just the base (i.e. no stat data), otherwise.
	 */
	if (res->status == PC_ITEM_WRITTEN) {
		assert_pc_item_result_size(len, (int)sizeof(struct pc_item_result));
		st = &res->st;
	} else {
		assert_pc_item_result_size(len, (int)PC_ITEM_RESULT_BASE_SIZE);
	}

	if (!worker->nr_items_to_complete)
		BUG("received result from supposedly finished checkout worker");
	if (res->id != worker->next_item_to_complete)
		BUG("unexpected item id from checkout worker (got %"PRIuMAX", exp %"PRIuMAX")",
		    (uintmax_t)res->id, (uintmax_t)worker->next_item_to_complete);

	worker->next_item_to_complete++;
	worker->nr_items_to_complete--;

	pc_item = &parallel_checkout.items[res->id];
	pc_item->status = res->status;
	if (st)
		pc_item->st = *st;

	if (res->status != PC_ITEM_COLLIDED)
		advance_progress_meter();
}


// Source: parallel-checkout.c
// Lines 535-575
