// Source: curl/lib/http_chunks.c
// Lines 90-92
void Curl_httpchunk_init(struct Curl_easy *data)
{
  struct connectdata *conn = data->conn;
  struct Curl_chunker *chunk = &conn->chunk;
  chunk->hexindex = 0;      /* start at 0 */
  chunk->state = CHUNK_HEX; /* we get hex first! */
  Curl_dyn_init(&conn->trailer, DYN_H1_TRAILER);
}
