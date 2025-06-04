session_group_find(const char *name)
{
	struct session_group	sg;

	sg.name = name;
	return (RB_FIND(session_groups, &session_groups, &sg));
}


// Source: session.c
// Lines 532-538
