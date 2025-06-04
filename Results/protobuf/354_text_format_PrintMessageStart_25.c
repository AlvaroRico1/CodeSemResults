  void PrintMessageStart(const Message& /*message*/, int /*field_index*/,
                         int /*field_count*/, bool single_line_mode,
                         BaseTextGenerator* generator) const override {
    // This is safe as only TextGenerator is used with
    // DebugStringFieldValuePrinter.
    TextGenerator* text_generator = static_cast<TextGenerator*>(generator);
    if (single_line_mode) {
      text_generator->PrintMaybeWithMarker(" ", "{ ");
    } else {
      text_generator->PrintMaybeWithMarker(" ", "{\n");
    }
  }
};


// Source: text_format.cc
// Lines 1526-1538
