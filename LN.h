//
// Created by kirat on 19.09.23.
//

#ifndef BACKLOG_ITMO_CPP_LN_KURATZERO_LN_H
#define BACKLOG_ITMO_CPP_LN_KURATZERO_LN_H

#include <string_view>

#include <cstdint>
#include <vector>

#define LN_TYPE_MAX UINT64_MAX
#define LN_TYPE uint64_t
#define LN_HEX_BLOCKS 16

class LN
{
  public:
	explicit LN(const char *);

	explicit LN(std::string_view);

	explicit LN(long long num = 0);

	LN(const LN &copy) = default;

	LN(LN &&moved) noexcept : ln_sign(moved.ln_sign), ln_nan(moved.ln_nan) { ln_num = moved.ln_num; }

	~LN() = default;

	explicit operator long long() const;

	explicit operator bool() const;

	LN &operator=(const LN &copy) = default;

	LN &operator=(LN &&moved) noexcept;

	bool operator<(const LN &r) const { return compare_to(r) == -1; };

	bool operator>(const LN &r) const { return compare_to(r) == 1; };

	bool operator<=(const LN &r) const { return compare_to(r) <= 0; };

	bool operator>=(const LN &r) const { return compare_to(r) >= 0; };

	bool operator==(const LN &r) const { return compare_to(r) == 0; };

	bool operator!=(const LN &r) const { return compare_to(r) != 0; };

	LN operator-();

	LN operator+(const LN &r) const;

	LN operator-(const LN &r) const;

	LN operator*(const LN &r) const;

	LN operator/(const LN &r) const;

	LN operator%(const LN &r) const;

	LN &operator++() { return operator+=(LN(1)); }

	LN &operator--() { return operator-=(LN(1)); }

	LN operator++(int);

	LN operator--(int);

	LN &operator+=(const LN &r);

	LN &operator-=(const LN &r);

	LN &operator*=(const LN &r);

	LN &operator/=(const LN &r);

	LN &operator%=(const LN &r);

	LN operator~();

	[[nodiscard]] std::string to_hex_string() const;

  private:
	bool ln_nan;
	bool ln_sign;
	std::vector< LN_TYPE > ln_num;

	void div_2();

	[[nodiscard]] int compare_to(const LN &) const;

	[[nodiscard]] bool is_null() const;

	void plus(const LN &r);

	void subs(const LN &r);

	void push_number(const char[LN_HEX_BLOCKS]);

	static char change_char(char c);

	static char change_num(char c);

	template< typename T >
	static bool check_is_number(int s_len, T s_num);
};

LN operator"" _ln(const char *s_num);

#endif	  // BACKLOG_ITMO_CPP_LN_KURATZERO_LN_H
