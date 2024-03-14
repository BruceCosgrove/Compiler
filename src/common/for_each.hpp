#pragma once

// https://www.scs.stanford.edu/~dm/blog/va-opt.html

#define EXPAND(...) __VA_ARGS__
#define PARENS ()
#define CALL(macro, ...) macro(__VA_ARGS__)

#define _FOR_EACH_EXPAND4(...) __VA_ARGS__
#define _FOR_EACH_EXPAND3(...) _FOR_EACH_EXPAND4(_FOR_EACH_EXPAND4(_FOR_EACH_EXPAND4(_FOR_EACH_EXPAND4(__VA_ARGS__))))
#define _FOR_EACH_EXPAND2(...) _FOR_EACH_EXPAND3(_FOR_EACH_EXPAND3(_FOR_EACH_EXPAND3(_FOR_EACH_EXPAND3(__VA_ARGS__))))
#define _FOR_EACH_EXPAND1(...) _FOR_EACH_EXPAND2(_FOR_EACH_EXPAND2(_FOR_EACH_EXPAND2(_FOR_EACH_EXPAND2(__VA_ARGS__))))
#define _FOR_EACH_EXPAND(...)  _FOR_EACH_EXPAND1(_FOR_EACH_EXPAND1(_FOR_EACH_EXPAND1(_FOR_EACH_EXPAND1(__VA_ARGS__))))

#define _FOR_EACH_HELPER(macro, arg, ...) macro(arg) __VA_OPT__(_FOR_EACH_AGAIN PARENS (macro, __VA_ARGS__))
#define _FOR_EACH_AGAIN() _FOR_EACH_HELPER
#define FOR_EACH(macro, ...) __VA_OPT__(_FOR_EACH_EXPAND(_FOR_EACH_HELPER(macro, __VA_ARGS__)))

#define _FOR_EACH_ARGS_HELPER(macro, args, arg, ...) macro(EXPAND args, arg) __VA_OPT__(_FOR_EACH_ARGS_AGAIN PARENS (macro, args, __VA_ARGS__))
#define _FOR_EACH_ARGS_AGAIN() _FOR_EACH_ARGS_HELPER
#define FOR_EACH_ARGS(macro, args, ...) __VA_OPT__(_FOR_EACH_EXPAND(_FOR_EACH_ARGS_HELPER(macro, args, __VA_ARGS__)))
