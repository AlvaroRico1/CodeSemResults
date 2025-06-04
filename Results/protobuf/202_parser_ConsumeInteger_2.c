bool Parser::ConsumeInteger(int* output, const char* error) {
  if (LookingAtType(io::Tokenizer::TYPE_INTEGER)) {
    uint64_t value = 0;
    if (!io::Tokenizer::ParseInteger(input_->current().text,
                                     std::numeric_limits<int32_t>::max(),
                                     &value)) {
      AddError("Integer out of range.");
      // We still return true because we did, in fact, parse an integer.
    }
    *output = value;
    input_->Next();
    return true;
  } else {
    AddError(error);
    return false;
  }


// Source: parser.cc
// Lines 239-254
