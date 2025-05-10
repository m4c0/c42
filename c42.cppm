export module c42;
import :phases13;
import :tokens;
import hai;
import jute;
import traits;

using namespace c42;
using namespace jute::literals;

class context {
  const char * m_buf;
  hai::chain<token> m_t; 

public:
  constexpr context(const char * buf, unsigned size)
    : m_buf { buf }
    , m_t { size }
  {}

  constexpr context(const char * buf, hai::chain<token> t)
    : m_buf { buf }
    , m_t { traits::move(t) }
  {}

  [[nodiscard]] context shallow() const { return context { m_buf, m_t.size() }; }
  [[nodiscard]] token_stream stream() const { return token_stream { m_t }; }
  [[nodiscard]] jute::view txt(token t) const {
    return jute::view { m_buf + t.begin, t.end - t.begin + 1 };
  }

  auto take() { return traits::move(m_t); }

  void push_back(token t) { m_t.push_back(t); }
};

static void consume_space(token_stream &str) {
  while (str.peek().type == t_space) {
    str.skip(1);
  }
}

/// Translates preprocessor directives (#, import, export) into custom tokens
static auto phase_4_1(const context & ctx) {
  context res = ctx.shallow();
  auto str = ctx.stream();
  while (str.has_more()) {
    consume_space(str);

    auto t = str.take();
    if (t.type == '#') {
      consume_space(str);

      if (!str.has_more()) break;

      t = str.take();
      t.type = t_directive;
      consume_space(str);
    } else if (t.type == t_import) {
      consume_space(str);
    } else if (t.type == t_module) {
      consume_space(str);
    }

    res.push_back(t);
    while (str.has_more() && t.type != t_new_line) {
      t = str.take();
      res.push_back(t);
    }
  }
  return res;
}

/// Process supported directives
static auto phase_4_2(const context & ctx) {
  context res = ctx.shallow();
  auto str = ctx.stream();
  while (str.has_more()) {
    auto t = str.take();

    if (t.type == t_directive) {
      auto txt = ctx.txt(t);
      if (txt == "error" || txt == "warning") {
        auto t = str.take();
        auto rt = t;
        auto nt = t;
        while (str.has_more() && t.type != t_new_line) {
          nt = t;
          t = str.take();
        }
        rt.type = txt == "error" ? t_error : t_warning;
        rt.end = nt.end;
        rt.value = ctx.txt(rt);
        res.push_back(rt);
        continue;
      } else if (txt == "embed") {
        while (str.has_more()) {
          consume_space(str);
          t = str.take();
          if (t.type == t_new_line) break;
          if (t.type != t_str) {
            auto nt = t;
            nt.type = t_error;
            nt.value = "Embeddable filenames must be strings"_hs;
            res.push_back(nt);
            continue;
          }
          t.type = t_error;
          t.value = "TBD - embed " + ctx.txt(t);
          res.push_back(t);
        }
        continue;
      }
    }

    res.push_back(t);
    while (str.has_more() && t.type != t_new_line) {
      t = str.take();
      res.push_back(t);
    }
  }
  return res;
}

static auto phase_4(const context & ctx) {
  return phase_4_2(phase_4_1(ctx));
}

export namespace c42 {
  auto preprocess(const hai::cstr & buf) {
    context ctx { buf.begin(), phase_3(phase_2(phase_1(buf))) };
    return phase_4(ctx).take();
  }
}
