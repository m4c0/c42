export module c42;
import hay;
import sv;
import traits;

namespace c42 {}

using namespace c42;

#include "c42-tokens.hpp"
#include "c42-phases13.hpp"
#include "c42-phase41.hpp"

export namespace c42 {
  struct defines {
    virtual bool has(sv name) const = 0;
  };
}

static void do_ifdef(bool take, defines * defs, token ot, token_stream & str, token_list & res) {
  while (str.has_more()) {
    auto t = str.take();
    switch (t.type) {
      case t_ifdef:
        do_ifdef(take && defs->has(res.txt(t)), defs, t, str, res);
        break;
      case t_ifndef:
        do_ifdef(take && !defs->has(res.txt(t)), defs, t, str, res);
        break;
      case t_else:
        do_ifdef(!take, defs, t, str, res);
        return;
      case t_endif:
        return;
      case t_elifdef:
        do_ifdef(!take && defs->has(res.txt(t)), defs, t, str, res);
        break;
      case t_elifndef:
        do_ifdef(!take && !defs->has(res.txt(t)), defs, t, str, res);
        break;
      default:
        if (take) res.push_back(t);
        break;
    }
  }
  ot.type = t_error;
  res.push_back(ot);
}

/// Process supported directives
static auto phase_4_2(defines * defs, const token_list & ctx) {
  auto res = ctx.shallow();
  token_stream str { ctx };
  while (str.has_more()) {
    auto t = str.take();

    switch (t.type) {
      case t_ifdef:
        do_ifdef(defs->has(ctx.txt(t)), defs, t, str, res);
        break;
      case t_ifndef:
        do_ifdef(!defs->has(ctx.txt(t)), defs, t, str, res);
        break;
      case t_else:
      case t_elifdef:
      case t_elifndef:
      case t_endif:
        // elifdef et al without ifdef
        t.type = t_error;
        break;
      default:
        res.push_back(t);
        break;
    }
  }
  return res;
}

static auto phase_4(defines * defs, const token_list & ctx) {
  return phase_4_2(defs, phase_4_1(ctx));
}

export namespace c42 {
  auto preprocess(defines * defs, sv buf) {
    return phase_4(defs, phase_3(phase_2(phase_1(buf))));
  }
}
