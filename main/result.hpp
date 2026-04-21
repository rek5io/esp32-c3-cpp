#pragma once

#include <optional>
#include <stdexcept>
#include <format>
#include <functional>

namespace result {
    template <typename T, typename E>
    class Result {
        private:
            std::optional<T> t = std::nullopt;
            std::optional<E> e = std::nullopt;

            Result() {}
        public:
            inline static auto Ok(T value) -> Result {
                Result r;
                r.t = std::move(value);
                return r;
            }

            inline static auto Err(E error) -> Result {
                Result r;
                r.e = std::move(error);
                return r;
            }

            inline auto is_ok() const -> bool {
                return t.has_value();
            }

            inline auto is_err() const -> bool {
                return e.has_value();
            }

            auto unwrap() const -> T {
                if (!t) {
                    throw std::runtime_error("Called unwrap on Err");
                }
                
                return std::move(t.value());
            }

            auto unwrap_err() const -> E {
                if (!e) {
                    throw std::runtime_error("Called unwrap_err on Ok");
                }
                
                return std::move(e.value());
            }

            auto unwrap_ref() -> T& {
                if (!t) {
                    throw std::runtime_error("Called unwrap_ref on Err");
                }
                
                return t.value();
            }

            auto unwrap_err_ref() -> E& {
                if (!e) {
                    throw std::runtime_error("Called unwrap_err_ref on Ok");
                }
                
                return e.value();
            }

            auto unwrap_ref() const -> const T& {
                if (!t) {
                    throw std::runtime_error("Called unwrap_ref on Err");
                }
                
                return t.value();
            }

            auto unwrap_err_ref() const -> const E& {
                if (!e) {
                    throw std::runtime_error("Called unwrap_err_ref on Ok");
                }
                
                return e.value();
            }

            inline auto unwrap_or(T fallback) -> T {
                return t.value_or(std::move(fallback));
            }

            template <typename F>
            void on_ok(F&& f) {
                if (!e) {
                    std::invoke(std::forward<F>(f), t.value());
                }
            }

            template <typename F>
            void on_err(F&& f) {
                if (!t) {
                    std::invoke(std::forward<F>(f), e.value());
                }
            }
    };
}

namespace std {
    template <typename T, typename E>
    struct formatter<result::Result<T, E>> {
        constexpr auto parse(format_parse_context& ctx) {
            return ctx.begin();
        }

        template <typename FormatContext>
        auto format(const result::Result<T, E>& r, FormatContext& ctx) const {
            if (r.is_ok()) {
                return format_to(ctx.out(), "Result::Ok({})", r.unwrap_ref());
            } else {
                return format_to(ctx.out(), "Result::Err({})", r.unwrap_err_ref());
            }
        }
    };
}
