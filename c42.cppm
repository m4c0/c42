export module c42;
import :phases13;
import :phase41;
import :tokens;
import hai;
import sv;
import traits;

/// Process supported directives
static auto phase_4_2(const context & ctx) {
  context res = ctx.shallow();
  auto str = ctx.stream();
  while (str.has_more()) {
    auto t = str.take();

    switch (t.type) {
      case t_ifdef:
      case t_ifndef:
        break;
      case t_else:
      case t_elifdef:
      case t_elifndef:
      case t_endif:
        // elifdef et al without ifdef
        t.type = t_error;
        break;
      default:
        break;
    }

    res.push_back(t);
  }
  return res;
}

static auto phase_4(const context & ctx) {
  return phase_4_2(phase_4_1(ctx));
}

export namespace c42 {
  auto preprocess(sv buf) {
    context ctx { buf.begin(), phase_3(phase_2(phase_1(buf))) };
    return phase_4(ctx);
  }
}
