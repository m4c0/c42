namespace c42 {
  export enum token_type : int {
    t_elif = -24,
    t_if = -23,
    t_elifndef = -22,
    t_elifdef = -21,
    t_ifndef = -20,
    t_ifdef = -19,
    t_define = -18,
    t_include = -17,
    t_else = -16,
    t_endif = -15,
    t_pragma = -14,
    t_export = -13,
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
    t_ex = -1,
    t_new_line = '\n',
    t_space = ' ',
  };
  // TODO: custom error messages
  export struct token {
    token_type type;
    unsigned begin;
    unsigned end;
    unsigned line;
    unsigned column;
  };

  export struct oob {};
  class token_list {
    sv m_orig_src;

    hay<token[], nullptr, nullptr> m_data;
    unsigned m_size = 0;
    unsigned m_capacity;
    
  public:
    constexpr token_list(sv orig_src, unsigned capacity) :
      m_orig_src { orig_src }
    , m_data { capacity }
    , m_capacity { capacity }
    {}

    // TODO: assert file is less than 2GB
    constexpr token_list(sv orig_src) : token_list { orig_src, static_cast<unsigned>(orig_src.size()) } {}

    [[nodiscard]] constexpr const auto * begin() const { return &m_data[0]; }
    [[nodiscard]] constexpr const auto * end() const { return &m_data[m_size]; }

    [[nodiscard]] constexpr auto size() const { return m_size; }
    [[nodiscard]] constexpr auto seek(unsigned n) const {
      return n >= m_size ? token {} : m_data[n];
    }

    [[nodiscard]] constexpr sv txt(token t) const {
      return m_orig_src.subview(t.begin, t.end - t.begin + 1).middle;
    }

    [[nodiscard]] constexpr auto shallow() const {
      return token_list { m_orig_src, m_size };
    }

    constexpr void push_back(token t) {
      if (m_size > m_capacity) throw oob {};
      m_data[m_size++] = t;
    }
  };

  class token_stream {
    const token_list & m_tokens;
    unsigned offset{};
  
    token eof() const {
      const auto &last = m_tokens.seek(m_tokens.size() - 1);
      return token{.type = t_eof, .begin = last.end + 1, .end = last.end + 1};
    }
  
  public:
    explicit token_stream(const token_list & t) : m_tokens(t) {}
  
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
