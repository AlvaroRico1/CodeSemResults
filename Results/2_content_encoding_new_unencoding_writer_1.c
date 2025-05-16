new_unencoding_writer(struct Curl_easy *data,
                      const struct content_encoding *handler,
                      struct contenc_writer *downstream)
{
  size_t sz = offsetof(struct contenc_writer, params) + handler->paramsize;
  struct contenc_writer *writer = (struct contenc_writer *)calloc(1, sz);

  if(writer) {
    writer->handler = handler;
    writer->downstream = downstream;
    if(handler->init_writer(data, writer)) {
      free(writer);
      writer = NULL;
    }
  }

  return writer;
}


// Source: content_encoding.c
// Lines 969-986
