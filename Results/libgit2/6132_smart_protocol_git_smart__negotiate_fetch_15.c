int git_smart__negotiate_fetch(git_transport *transport, git_repository *repo, const git_remote_head * const *wants, size_t count)
{
	transport_smart *t = (transport_smart *)transport;
	git_revwalk__push_options opts = GIT_REVWALK__PUSH_OPTIONS_INIT;
	gitno_buffer *buf = &t->buffer;
	git_str data = GIT_STR_INIT;
	git_revwalk *walk = NULL;
	int error = -1;
	git_pkt_type pkt_type;
	unsigned int i;
	git_oid oid;

	if ((error = git_pkt_buffer_wants(wants, count, &t->caps, &data)) < 0)
		return error;

	if ((error = git_revwalk_new(&walk, repo)) < 0)
		goto on_error;

	opts.insert_by_date = 1;
	if ((error = git_revwalk__push_glob(walk, "refs/*", &opts)) < 0)
		goto on_error;

	/*
	 * Our support for ACK extensions is simply to parse them. On
	 * the first ACK we will accept that as enough common
	 * objects. We give up if we haven't found an answer in the
	 * first 256 we send.
	 */
	i = 0;
	while (i < 256) {
		error = git_revwalk_next(&oid, walk);

		if (error < 0) {
			if (GIT_ITEROVER == error)
				break;

			goto on_error;
		}

		git_pkt_buffer_have(&oid, &data);
		i++;
		if (i % 20 == 0) {
			if (t->cancelled.val) {
				git_error_set(GIT_ERROR_NET, "The fetch was cancelled by the user");
				error = GIT_EUSER;
				goto on_error;
			}

			git_pkt_buffer_flush(&data);
			if (git_str_oom(&data)) {
				error = -1;
				goto on_error;
			}

			if ((error = git_smart__negotiation_step(&t->parent, data.ptr, data.size)) < 0)
				goto on_error;

			git_str_clear(&data);
			if (t->caps.multi_ack || t->caps.multi_ack_detailed) {
				if ((error = store_common(t)) < 0)
					goto on_error;
			} else {
				if ((error = recv_pkt(NULL, &pkt_type, buf)) < 0)
					goto on_error;

				if (pkt_type == GIT_PKT_ACK) {
					break;
				} else if (pkt_type == GIT_PKT_NAK) {
					continue;
				} else {
					git_error_set(GIT_ERROR_NET, "unexpected pkt type");
					error = -1;
					goto on_error;
				}
			}
		}

		if (t->common.length > 0)
			break;

		if (i % 20 == 0 && t->rpc) {
			git_pkt_ack *pkt;
			unsigned int j;

			if ((error = git_pkt_buffer_wants(wants, count, &t->caps, &data)) < 0)
				goto on_error;

			git_vector_foreach(&t->common, j, pkt) {
				if ((error = git_pkt_buffer_have(&pkt->oid, &data)) < 0)
					goto on_error;
			}

			if (git_str_oom(&data)) {
				error = -1;
				goto on_error;
			}
		}
	}

	/* Tell the other end that we're done negotiating */
	if (t->rpc && t->common.length > 0) {
		git_pkt_ack *pkt;
		unsigned int j;

		if ((error = git_pkt_buffer_wants(wants, count, &t->caps, &data)) < 0)
			goto on_error;

		git_vector_foreach(&t->common, j, pkt) {
			if ((error = git_pkt_buffer_have(&pkt->oid, &data)) < 0)
				goto on_error;
		}

		if (git_str_oom(&data)) {
			error = -1;
			goto on_error;
		}
	}

	if ((error = git_pkt_buffer_done(&data)) < 0)
		goto on_error;

	if (t->cancelled.val) {
		git_error_set(GIT_ERROR_NET, "The fetch was cancelled by the user");
		error = GIT_EUSER;
		goto on_error;
	}
	if ((error = git_smart__negotiation_step(&t->parent, data.ptr, data.size)) < 0)
		goto on_error;

	git_str_dispose(&data);
	git_revwalk_free(walk);

	/* Now let's eat up whatever the server gives us */
	if (!t->caps.multi_ack && !t->caps.multi_ack_detailed) {
		if ((error = recv_pkt(NULL, &pkt_type, buf)) < 0)
			return error;

		if (pkt_type != GIT_PKT_ACK && pkt_type != GIT_PKT_NAK) {
			git_error_set(GIT_ERROR_NET, "unexpected pkt type");
			return -1;
		}
	} else {
		error = wait_while_ack(buf);
	}

	return error;

on_error:
	git_revwalk_free(walk);
	git_str_dispose(&data);
	return error;
}


// Source: smart_protocol.c
// Lines 320-471
