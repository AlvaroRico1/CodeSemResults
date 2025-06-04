environ_find(struct environ *env, const char *name)
{
	struct environ_entry	envent;

	envent.name = (char *) name;
	return (RB_FIND(environ, env, &envent));
}


// Source: environ.c
// Lines 99-105
