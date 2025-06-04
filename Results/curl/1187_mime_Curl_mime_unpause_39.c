void Curl_mime_unpause(curl_mimepart *part)
{
  if(part) {
    if(part->lastreadstatus == CURL_READFUNC_PAUSE)
      part->lastreadstatus = 1; /* Successful read status. */
    if(part->kind == MIMEKIND_MULTIPART) {
      curl_mime *mime = (curl_mime *) part->arg;

      if(mime) {
        curl_mimepart *subpart;

        for(subpart = mime->firstpart; subpart; subpart = subpart->nextpart)
          Curl_mime_unpause(subpart);
      }
    }


// Source: mime.c
// Lines 1938-1952
