static void dos_time(timestamp_t *timestamp, int *dos_date, int *dos_time)
{
	time_t time;
	struct tm tm;

	if (date_overflows(*timestamp))
		die(_("timestamp too large for this system: %"PRItime),
		    *timestamp);
	time = (time_t)*timestamp;
	localtime_r(&time, &tm);
	*timestamp = time;

	*dos_date = tm.tm_mday + (tm.tm_mon + 1) * 32 +
		    (tm.tm_year + 1900 - 1980) * 512;
	*dos_time = tm.tm_sec / 2 + tm.tm_min * 32 + tm.tm_hour * 2048;
}


// Source: archive-zip.c
// Lines 597-612
