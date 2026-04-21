#pragma once

#include <format>

namespace unit {
    struct Unit {};
}

namespace std {
    template <>
    struct formatter<unit::Unit> {
        constexpr auto parse(format_parse_context& ctx) {
            return ctx.begin();
        }

        template <typename FormatContext>
        auto format(const unit::Unit& r, FormatContext& ctx) const {
            return format_to(ctx.out(), "Unit");
        }
    };
}
