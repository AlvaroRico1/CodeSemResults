void Query_block::merge_contexts(Query_block *inner) {
  for (Name_resolution_context *ctx = inner->first_context; ctx != nullptr;
       ctx = ctx->next_context) {
    ctx->query_block = this;
    if (ctx->next_context == nullptr) {
      ctx->next_context = first_context;
      first_context = inner->first_context;
      inner->first_context = nullptr;
      break;
    }
  }
}


// Source: sql_resolver.cc
// Lines 4028-4039
