bool Parser::ConsumeNumber(double* output, const char* error) {
  if (LookingAtType(io::Tokenizer::TYPE_FLOAT)) {
    *output = io::Tokenizer::ParseFloat(input_->current().text);
    input_->Next();
    return true;
  } else if (LookingAtType(io::Tokenizer::TYPE_INTEGER)) {
    // Also accept integers.
    uint64_t value = 0;
    if (!io::Tokenizer::ParseInteger(input_->current().text,
                                     std::numeric_limits<uint64_t>::max(),
                                     &value)) {
      AddError("Integer out of range.");
      // We still return true because we did, in fact, parse a number.
    }
    *output = value;
    input_->Next();
    return true;
  } else if (LookingAt("inf")) {
    *output = std::numeric_limits<double>::infinity();
    input_->Next();
    return true;
  } else if (LookingAt("nan")) {
    *output = std::numeric_limits<double>::quiet_NaN();
    input_->Next();
    return true;
  } else {
    AddError(error);
    return false;
  }


// Source: parser.cc
// Lines 288-316
