  void InvalidValue(const converter::LocationTrackerInterface& loc,
                    StringPiece type_name,
                    StringPiece value) override {
    status_ = util::InvalidArgumentError(
        StrCat(GetLocString(loc), ": invalid value ", std::string(value),
                     " for type ", std::string(type_name)));
  }

  void MissingField(const converter::LocationTrackerInterface& loc,
                    StringPiece missing_name) override {
    status_ = util::InvalidArgumentError(StrCat(
        GetLocString(loc), ": missing field ", std::string(missing_name)));
  }

 private:
  util::Status status_;

  std::string GetLocString(const converter::LocationTrackerInterface& loc) {
    std::string loc_string = loc.ToString();
    StripWhitespace(&loc_string);
    if (!loc_string.empty()) {
      loc_string = StrCat("(", loc_string, ")");
    }
    return loc_string;
  }

  GOOGLE_DISALLOW_EVIL_CONSTRUCTORS(StatusErrorListener);
};


// Source: json_util.cc
// Lines 147-174
