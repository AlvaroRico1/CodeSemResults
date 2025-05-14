// Source: curl/src/tool_cb_prg.c
// Lines 124-132
int tool_progress_cb(void *clientp,
                     curl_off_t dltotal, curl_off_t dlnow,
                     curl_off_t ultotal, curl_off_t ulnow)
{
  /* The original progress-bar source code was written for curl by Lars Aas,
     and this new edition inherits some of his concepts. */

  struct timeval now = tvnow();
  struct per_transfer *per = clientp;
  struct OperationConfig *config = per->config;
  struct ProgressData *bar = &per->progressbar;
  curl_off_t total;
  curl_off_t point;

  /* Calculate expected transfer size. initial_size can be less than zero
     when indicating that we are expecting to get the filesize from the
     remote */
  if(bar->initial_size < 0 ||
     ((CURL_OFF_T_MAX - bar->initial_size) < (dltotal + ultotal)))
    total = CURL_OFF_T_MAX;
  else
    total = dltotal + ultotal + bar->initial_size;

  /* Calculate the current progress. initial_size can be less than zero when
     indicating that we are expecting to get the filesize from the remote */
  if(bar->initial_size < 0 ||
     ((CURL_OFF_T_MAX - bar->initial_size) < (dlnow + ulnow)))
    point = CURL_OFF_T_MAX;
  else
    point = dlnow + ulnow + bar->initial_size;

  if(bar->calls) {
    /* after first call... */
    if(total) {
      /* we know the total data to get... */
      if(bar->prev == point)
        /* progress didn't change since last invoke */
        return 0;
      else if((tvdiff(now, bar->prevtime) < 100L) && point < total)
        /* limit progress-bar updating to 10 Hz except when we're at 100% */
        return 0;
    }
    else {
      /* total is unknown */
      if(tvdiff(now, bar->prevtime) < 100L)
        /* limit progress-bar updating to 10 Hz */
        return 0;
      fly(bar, point != bar->prev);
    }
  }

  /* simply count invokes */
  bar->calls++;

  if((total > 0) && (point != bar->prev)) {
    char line[MAX_BARLENGTH + 1];
    char format[40];
    double frac;
    double percent;
    int barwidth;
    int num;
    if(point > total)
      /* we have got more than the expected total! */
      total = point;

    frac = (double)point / (double)total;
    percent = frac * 100.0;
    barwidth = bar->width - 7;
    num = (int) (((double)barwidth) * frac);
    if(num > MAX_BARLENGTH)
      num = MAX_BARLENGTH;
    memset(line, '#', num);
    line[num] = '\0';
    msnprintf(format, sizeof(format), "\r%%-%ds %%5.1f%%%%", barwidth);
    fprintf(bar->out, format, line, percent);
  }
  fflush(bar->out);
  bar->prev = point;
  bar->prevtime = now;

  if(config->readbusy) {
    config->readbusy = FALSE;
    curl_easy_pause(per->curl, CURLPAUSE_CONT);
  }

  return 0;
}
