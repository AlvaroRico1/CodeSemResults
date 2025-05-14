// Source: curl/lib/connect.c
// Lines 563-563
static struct Curl_addrinfo *ainext(struct connectdata *conn,
                                    int tempindex,
                                    bool next) /* use next entry? */
{
  struct Curl_addrinfo *ai = conn->tempaddr[tempindex];
  if(ai && next)
    ai = ai->ai_next;
  while(ai && (ai->ai_family != conn->tempfamily[tempindex]))
    ai = ai->ai_next;
  conn->tempaddr[tempindex] = ai;
  return ai;
}
