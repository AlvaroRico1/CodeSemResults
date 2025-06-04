int dyn_event_release(int argc, char **argv, struct dyn_event_operations *type)
{
	struct dyn_event *pos, *n;
	char *system = NULL, *event, *p;
	int ret = -ENOENT;

	if (argv[0][0] == '-') {
		if (argv[0][1] != ':')
			return -EINVAL;
		event = &argv[0][2];
	} else {
		event = strchr(argv[0], ':');
		if (!event)
			return -EINVAL;
		event++;
	}

	p = strchr(event, '/');
	if (p) {
		system = event;
		event = p + 1;
		*p = '\0';
	}
	if (event[0] == '\0')
		return -EINVAL;

	mutex_lock(&event_mutex);
	for_each_dyn_event_safe(pos, n) {
		if (type && type != pos->ops)
			continue;
		if (pos->ops->match(system, event, pos)) {
			ret = pos->ops->free(pos);
			break;
		}
	}
	mutex_unlock(&event_mutex);

	return ret;
}


// Source: trace_dynevent.c
// Lines 34-72
