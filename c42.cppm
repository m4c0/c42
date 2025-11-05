export module c42;
import :phases13;
import :tokens;
import hai;
import jute;
import sv;
import traits;

using namespace c42;
using namespace jute::literals;

class context {
  const char * m_orig_src;
  hai::chain<token> m_t; 

public:
  constexpr context(const char * orig_src, unsigned tk_buf_size)
    : m_orig_src { orig_src }
    , m_t { tk_buf_size }
  {}

  constexpr context(const char * orig_src, hai::chain<token> t)
    : m_orig_src { orig_src }
    , m_t { traits::move(t) }
  {}

  [[nodiscard]] context shallow() const { return context { m_orig_src, m_t.size() }; }
  [[nodiscard]] token_stream stream() const { return token_stream { m_t }; }
  [[nodiscard]] jute::view txt(token t) const {
    return jute::view { m_orig_src + t.begin, t.end - t.begin + 1 };
  }

  auto take() { return traits::move(m_t); }

  void push_back(token t) { m_t.push_back(t); }
};

static void consume_space(token_stream &str) {
  while (str.peek().type == t_space) {
    str.skip(1);
  }
}

static auto take_until_eol(const context & ctx, token_stream & str, token_type type) {
  auto t = str.take();
  auto rt = t;
  auto nt = t;
  while (str.has_more() && t.type != t_new_line) {
    nt = t;
    t = str.take();
  }
  if (t.type == t_new_line) t = str.take();
  rt.type = type;
  rt.end = nt.end;
  return rt;
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
      if (t.type != t_identifier) {
        // TODO: error message
        t.type = t_error;
      } else if (ctx.txt(t) == "error") {
        consume_space(str);
        res.push_back(take_until_eol(ctx, str, t_error));
        continue;
      } else if (ctx.txt(t) == "pragma") {
        consume_space(str);
        res.push_back(take_until_eol(ctx, str, t_pragma));
        continue;
      } else if (ctx.txt(t) == "warning") {
        consume_space(str);
        res.push_back(take_until_eol(ctx, str, t_warning));
        continue;
      } else {
        t.type = t_directive;
        consume_space(str);
      }
    } else if (t.type == t_identifier && ctx.txt(t) == "export") {
      t.type = t_export;
      res.push_back(t);
      continue; // process next token as if it wasn't exported
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
  auto preprocess(sv buf) {
    context ctx { buf.begin(), phase_3(phase_2(phase_1(buf))) };
    return phase_4(ctx).take();
  }
}
