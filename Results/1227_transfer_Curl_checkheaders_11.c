char *Curl_checkheaders(const struct Curl_easy *data,
                        const char *thisheader)
{
  struct curl_slist *head;
  size_t thislen = strlen(thisheader);
  DEBUGASSERT(thislen);
  DEBUGASSERT(thisheader[thislen-1] != ':');

  for(head = data->set.headers; head; head = head->next) {
    if(strncasecompare(head->data, thisheader, thislen) &&
       Curl_headersep(head->data[thislen]) )
      return head->data;
  }

  return NULL;
}


// Source: transfer.c
// Lines 97-112
