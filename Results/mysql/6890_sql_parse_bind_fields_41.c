void bind_fields(Item *first) {
  for (Item *item = first; item; item = item->next_free) item->bind_fields();
}


// Source: sql_parse.cc
// Lines 1188-1190
