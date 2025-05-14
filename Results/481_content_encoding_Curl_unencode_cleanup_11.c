// Source: curl/lib/content_encoding.c
// Lines 1002-1003
void Curl_unencode_cleanup(struct Curl_easy *data)
{
  struct SingleRequest *k = &data->req;
  struct contenc_writer *writer = k->writer_stack;

  while(writer) {
    k->writer_stack = writer->downstream;
    writer->handler->close_writer(data, writer);
    free(writer);
    writer = k->writer_stack;
  }
}
