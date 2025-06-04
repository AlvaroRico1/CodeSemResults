int git_signature_now(git_signature **sig_out, const char *name, const char *email)
{
	time_t now;
	time_t offset;
	struct tm *utc_tm;
	git_signature *sig;
	struct tm _utc;

	*sig_out = NULL;

	/*
	 * Get the current time as seconds since the epoch and
	 * transform that into a tm struct containing the time at
	 * UTC. Give that to mktime which considers it a local time
	 * (tm_isdst = -1 asks it to take DST into account) and gives
	 * us that time as seconds since the epoch. The difference
	 * between its return value and 'now' is our offset to UTC.
	 */
	time(&now);
	utc_tm = p_gmtime_r(&now, &_utc);
	utc_tm->tm_isdst = -1;
	offset = (time_t)difftime(now, mktime(utc_tm));
	offset /= 60;

	if (git_signature_new(&sig, name, email, now, (int)offset) < 0)
		return -1;

	*sig_out = sig;

	return 0;
}


// Source: signature.c
// Lines 156-186
