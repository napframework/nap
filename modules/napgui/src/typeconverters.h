#include <nap/stringutils.h>
#include "layoutcomponent.h"

bool conv_rect_to_string(const nap::Rect &inval, std::string &outval);

bool conv_string_to_rect(const std::string& inval, nap::Rect& outval);

bool conv_margins_to_string(const nap::Margins& inval, std::string& outval);

bool conv_string_to_margins(const std::string& inval, nap::Margins& outval);