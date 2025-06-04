bool Parser::ConsumeInteger64(uint64_t max_value, uint64_t* output,
                              const char* error) {
  if (LookingAtType(io::Tokenizer::TYPE_INTEGER)) {
    if (!io::Tokenizer::ParseInteger(input_->current().text, max_value,
                                     output)) {
      AddError("Integer out of range.");
      // We still return true because we did, in fact, parse an integer.
      *output = 0;
    }
    input_->Next();
    return true;
  } else {
    AddError(error);
    return false;
  }
}


// Source: parser.cc
// Lines 271-286
