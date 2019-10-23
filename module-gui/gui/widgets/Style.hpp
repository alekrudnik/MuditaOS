#pragma once

#include <inttypes.h>
#include <Label.hpp>
#include <string>

namespace style {
const inline uint32_t window_height = 600;
const inline uint32_t window_width  = 480;
namespace header {
namespace font {
const inline std::string time  = "gt_pressura_regular_24";
const inline std::string title = "gt_pressura_bold_32";
};  // namespace font
};  // namespace header
namespace window {
namespace font {
const inline std::string verybig = "gt_pressura_light_46";
const inline std::string bigbold = "gt_pressura_bold_30";
const inline std::string big     = "gt_pressura_regular_30";
const inline std::string medium  = "gt_pressura_regular_27";
const inline std::string small   = "gt_pressura_regular_24";
};  // namespace font

namespace label {
const inline uint32_t default_h         = 50;
const inline uint32_t border_focucs_w   = 3;
const inline uint32_t border_no_focus_w = 0;
};  // namespace label

/// minimal label decoration - edges & focus
void decorate(gui::Label *el);
/// minimal label decoration for Option
void decorateOption(gui::Label *el);

};  // namespace window
namespace footer {
namespace font {
const inline std::string bold   = "gt_pressura_bold_24";
const inline std::string medium = "gt_pressura_regular_24";
};  // namespace font
};  // namespace footer
};  // namespace style
