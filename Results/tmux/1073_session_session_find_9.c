session_find(const char *name)
{
	struct session	s;

	s.name = (char *) name;
	return (RB_FIND(sessions, &sessions, &s));
}


// Source: session.c
// Lines 75-81
