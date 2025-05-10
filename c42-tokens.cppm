export module c42:tokens;
import jute;
import hai;

namespace c42 {
  export enum token_type : int {
    t_warning = -12,
    t_error = -11,
    t_directive = -10,
    t_module = -9,
    t_import = -8,
    t_pp_number = -7,
    t_raw_str = -6,
    t_str = -5,
    t_char = -4,
    t_identifier = -3,
    t_eof = -2,
    t_null = -1,
    t_new_line = '\n',
    t_space = ' ',
  };
  export struct token {
    token_type type;
    jute::heap value {};
    unsigned begin;
    unsigned end;
    unsigned line;
    unsigned column;
  };

  class token_stream {
    const hai::chain<token> & m_tokens;
    unsigned offset{};
  
    token eof() const {
      const auto &last = m_tokens.seek(m_tokens.size() - 1);
      return token{.type = t_eof, .begin = last.end + 1, .end = last.end + 1};
    }
  
  public:
    explicit token_stream(const hai::chain<token> & t) : m_tokens(t) {}
  
    bool has_more() { return offset < m_tokens.size(); }
  
    void skip(unsigned n) {
      offset = (offset + n >= m_tokens.size()) ? m_tokens.size() : offset + n;
    }
    [[nodiscard]] token take() {
      if (offset >= m_tokens.size()) return eof();
      return m_tokens.seek(offset++);
    }
    [[nodiscard]] token peek(unsigned d = 0) const {
      if (offset + d >= m_tokens.size()) return eof();
      return m_tokens.seek(offset + d);
    }
  
    [[nodiscard]] bool matches(const char *txt) const {
      for (auto i = 0; txt[i] != 0; i++) {
        if (peek(i).type != txt[i])
          return false;
      }
      return true;
    }
  };
} 
