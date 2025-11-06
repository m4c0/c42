export module c42;
import :phases13;
import :phase41;
import :tokens;
import sv;

static void do_ifdef(bool take, token ot, token_stream & str, context & res) {
  while (str.has_more()) {
    auto t = str.take();
    switch (t.type) {
      case t_ifdef:
        // TODO: interpret the define
        do_ifdef(take && true, t, str, res);
        break;
      case t_ifndef:
        // TODO: interpret the define
        do_ifdef(take && false, t, str, res);
        break;
      case t_else:
        do_ifdef(!take, t, str, res);
        return;
      case t_endif:
        return;
      case t_elifdef:
        // TODO: interpret the define
        do_ifdef(!take && true, t, str, res);
        break;
      case t_elifndef:
        // TODO: interpret the define
        do_ifdef(!take && false, t, str, res);
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
static auto phase_4_2(const context & ctx) {
  context res = ctx.shallow();
  token_stream str { ctx };
  while (str.has_more()) {
    auto t = str.take();

    switch (t.type) {
      case t_ifdef:
        // TODO: interpret the define
        do_ifdef(true, t, str, res);
        break;
      case t_ifndef:
        // TODO: interpret the define
        do_ifdef(false, t, str, res);
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

static auto phase_4(const context & ctx) {
  return phase_4_2(phase_4_1(ctx));
}

export namespace c42 {
  auto preprocess(sv buf) {
    context ctx { buf.begin(), phase_3(phase_2(phase_1(buf))) };
    return phase_4(ctx);
  }
}
