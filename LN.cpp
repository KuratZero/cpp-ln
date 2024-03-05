//
// Created by kirat on 19.09.23.
//

#include "LN.h"

#include <algorithm>
#include <climits>
#include <cstring>
#include <iostream>
#include <vector>

LN::LN(long long num) : ln_nan(false), ln_sign(num < 0)
{
	if (num < 0)
	{
		num = ~num + 1;
	}
	ln_num.push_back(num);
}

LN::LN(const char *s_num)
{
	int s_len = strlen(s_num);
	if (s_len == 0)
	{
		ln_nan = true;
		ln_sign = false;
		return;
	}
	ln_nan = !check_is_number(s_len, s_num);
	if (ln_nan)
	{
		ln_sign = false;
		return;
	}
	ln_sign = (s_num[0] == '-');

	char buff[LN_HEX_BLOCKS];
	for (int i = s_len - 1; i >= 0; i -= LN_HEX_BLOCKS)
	{
		for (int j = 0; j < LN_HEX_BLOCKS; j++)
		{
			char c = 0;
			if (i - j >= 0)
			{
				c = change_char(s_num[i - j]);
			}
			buff[LN_HEX_BLOCKS - j - 1] = c;
		}
		push_number(buff);
	}
}

LN::LN(std::string_view s_num)
{
	if (s_num.empty())
	{
		ln_nan = true;
		ln_sign = false;
		return;
	}
	ln_nan = !check_is_number(s_num.length(), s_num);
	if (ln_nan)
	{
		ln_sign = false;
		return;
	}
	ln_sign = (s_num[0] == '-');

	char buff[LN_HEX_BLOCKS];
	for (int i = s_num.length() - 1; i >= 0; i -= LN_HEX_BLOCKS)
	{
		for (int j = 0; j < LN_HEX_BLOCKS; j++)
		{
			char c = 0;
			if (i - j >= 0)
			{
				c = change_char(s_num[i - j]);
			}
			buff[LN_HEX_BLOCKS - j - 1] = c;
		}
		push_number(buff);
	}
}

std::string LN::to_hex_string() const
{
	std::string tmp;
	if (ln_nan)
	{
		return "NaN";
	}
	for (LN_TYPE num : ln_num)
	{
		for (int i = 0; i < LN_HEX_BLOCKS; i++)
		{
			LN_TYPE c = (num >> (4 * (i))) % (1 << 4);
			tmp.push_back(change_num((char)c));
		}
	}
	while (!tmp.empty() && tmp.back() == '0')
	{
		tmp.pop_back();
	}
	if (tmp.empty())
	{
		return "0";
	}
	if (ln_sign)
	{
		tmp.push_back('-');
	}
	std::string result(tmp.rbegin(), tmp.rend());
	return result;
}

// - OPERATIONS -

LN::operator long long() const
{
	if (ln_nan)
	{
		throw std::invalid_argument("LN number is NaN.");
	}
	int r = 0;
	for (LN_TYPE t : ln_num)
	{
		if (t != 0)
		{
			r++;
		}
	}
	if (r > 1)
	{
		throw std::overflow_error("LN number overflows long long.");
	}
	if (is_null())
	{
		return 0;
	}
	if (ln_sign)
	{
		if (ln_num[0] <= (LN_TYPE)(LLONG_MAX) + 1)
		{
			return (long long)-ln_num[0];
		}
		throw std::overflow_error("LN number overflows long long.");
	}
	else
	{
		if (ln_num[0] <= (LN_TYPE)(LLONG_MAX))
		{
			return (long long)ln_num[0];
		}
		throw std::overflow_error("LN number overflows long long.");
	}
}

LN::operator bool() const
{
	if (ln_nan)
	{
		throw std::invalid_argument("LN number is NaN.");
	}
	return !is_null();
}

LN &LN::operator=(LN &&moved) noexcept
{
	ln_nan = moved.ln_nan;
	ln_sign = moved.ln_sign;
	ln_num = moved.ln_num;
	return *this;
}

LN LN::operator-()
{
	LN tmp(*this);
	tmp.ln_sign = !tmp.ln_sign;
	return tmp;
}

LN LN::operator++(int)
{
	const LN t(*this);
	++(*this);
	return t;
}

LN LN::operator--(int)
{
	const LN t(*this);
	--(*this);
	return t;
}

LN &LN::operator+=(const LN &r)
{
	if (r.ln_nan || ln_nan)
	{
		ln_nan = true;
		return *this;
	}

	if (ln_sign == r.ln_sign)
	{
		plus(r);
	}
	else
	{
		ln_sign = !ln_sign;
		operator-=(r);
		ln_sign = !ln_sign;
	}

	return *this;
}

LN &LN::operator-=(const LN &r)
{
	if (r.ln_nan || ln_nan)
	{
		ln_nan = true;
		return *this;
	}
	if (ln_sign != r.ln_sign)
	{
		plus(r);
	}
	else if ((!ln_sign && r > (*this)) || (ln_sign && r < (*this)))
	{
		LN tmp(r);
		tmp -= (*this);
		operator=(std::move(tmp));
		ln_sign = !ln_sign;
	}
	else
	{
		subs(r);
	}

	return *this;
}

LN &LN::operator*=(const LN &r)
{
	if (r.ln_nan || ln_nan)
	{
		ln_nan = true;
		return *this;
	}
	LN tmp(0ll);

	for (LN_TYPE i : r.ln_num)
	{
		for (size_t j = 0; j < LN_HEX_BLOCKS * 4; j++)
		{
			if ((i >> j) % 2)
			{
				tmp += *this;
			}
			*this += *this;	   // this = 2 * this = this << 1
		}
	}
	operator=(std::move(tmp));
	ln_sign = (ln_sign != r.ln_sign);
	return *this;
}

LN &LN::operator/=(const LN &r)
{
	if (r.ln_nan || ln_nan || r.is_null())
	{
		ln_nan = true;
		return *this;
	}
	if (ln_sign || r.ln_sign)
	{
		LN t1(*this);
		LN t2(r);
		if (t1.ln_sign)
			t1 = -t1;
		if (t2.ln_sign)
			t2 = -t2;
		t1 /= t2;
		t1.ln_sign = r.ln_sign != ln_sign;
		operator=(std::move(t1));
		return *this;
	}
	LN right(1ll);
	while ((r * right) <= *this)
	{
		right *= LN(2ll);
	}
	LN left(right);
	left.div_2();

	while ((right - left) > LN(1))
	{
		LN mid(left);
		mid += right;
		mid.div_2();
		if ((r * mid) <= *this)
		{
			left = std::move(mid);
		}
		else
		{
			right = std::move(mid);
		}
	}
	operator=(std::move(left));
	return *this;
}

LN &LN::operator%=(const LN &r)
{
	if (r.ln_nan || ln_nan || r.is_null())
	{
		ln_nan = true;
		return *this;
	}
	LN result_div = *this / r;
	operator-=(result_div *r);
	return *this;
}

LN LN::operator~()
{
	LN tmp(*this);
	if (tmp.ln_nan || tmp < LN(0ll))
	{
		tmp.ln_nan = true;
		return tmp;
	}
	LN left(0ll);
	LN right(tmp + LN(1));
	while ((right - left) > LN(1))
	{
		LN mid(left);
		mid += right;
		mid.div_2();
		if ((mid * mid) <= tmp)
		{
			left = std::move(mid);
		}
		else
		{
			right = std::move(mid);
		}
	}
	tmp = std::move(left);
	return tmp;
}

LN LN::operator+(const LN &r) const
{
	LN t(*this);
	t += r;
	return t;
}

LN LN::operator-(const LN &r) const
{
	LN t(*this);
	t -= r;
	return t;
}

LN LN::operator*(const LN &r) const
{
	LN t(*this);
	t *= r;
	return t;
}

LN LN::operator/(const LN &r) const
{
	LN t(*this);
	t /= r;
	return t;
}

LN LN::operator%(const LN &r) const
{
	LN t(*this);
	t %= r;
	return t;
}

LN operator""_ln(const char *s_num)
{
	return LN(s_num);
}

// - UTILS -

void LN::plus(const LN &r)
{
	int extra = 0;
	size_t maxi = std::max(ln_num.size(), r.ln_num.size());
	for (size_t i = 0; i < maxi || extra; i++)
	{
		if (i == ln_num.size())
			ln_num.push_back(0);

		LN_TYPE number = (i < r.ln_num.size() ? r.ln_num[i] : 0);
		int tt = extra;
		if ((extra && LN_TYPE_MAX - ln_num[i] <= number) || (LN_TYPE_MAX - ln_num[i] - extra < number))
			extra = 1;
		else
			extra = 0;

		ln_num[i] += tt + number;
	}
}

void LN::subs(const LN &r)
{
	int extra = 0;
	for (size_t i = 0; i < r.ln_num.size() || extra; i++)
	{
		int tt = extra;
		if ((tt && ln_num[i] <= r.ln_num[i]) || (ln_num[i] < tt + r.ln_num[i]))
			extra = 1;
		else
			extra = 0;

		ln_num[i] -= tt + (i < r.ln_num.size() ? r.ln_num[i] : 0);
	}
}

void LN::push_number(const char *nums)
{
	LN_TYPE num = 0;
	for (int i = LN_HEX_BLOCKS - 1; i >= 0; i--)
	{
		num |= ((LN_TYPE)(nums[i])) << ((LN_HEX_BLOCKS - i - 1) * 4);
	}
	ln_num.push_back(num);
}

char LN::change_char(char c)
{
	char c_ans = 0;
	if (c != '+' && c != '-')
	{
		if (std::isdigit(c))
		{
			c_ans = (char)((int)c - (int)'0');
		}
		else
		{
			c_ans = (char)((int)std::toupper(c) - (int)'A' + 10);
		}
	}
	return c_ans;
}

char LN::change_num(char c)
{
	if (c <= 9)
	{
		return (char)((int)c + (int)'0');
	}
	else
	{
		return (char)((int)c + (int)'A' - 10);
	}
}

int LN::compare_to(const LN &r) const
{
	// l < r == -1 | l == r == 0 | l > r == 1
	if (ln_nan || r.ln_nan)
	{
		return 0;
	}
	bool null_1 = is_null();
	bool null_2 = r.is_null();
	if (null_1 && null_2)
		return 0;
	if (null_1)
	{
		if (r.ln_sign)
			return 1;
		return -1;
	}
	if (null_2)
	{
		if (ln_sign)
			return -1;
		return 1;
	}
	if (!ln_sign && r.ln_sign)
	{
		return 1;
	}
	if (ln_sign && !r.ln_sign)
	{
		return -1;
	}

	size_t mini = std::min(ln_num.size(), r.ln_num.size());
	for (size_t i = ln_num.size(); i > mini; i--)
	{
		if (ln_num[i - 1] != 0)
		{
			return ln_sign ? -1 : 1;
		}
	}

	for (size_t i = r.ln_num.size(); i > mini; i--)
	{
		if (r.ln_num[i - 1] != 0)
		{
			return r.ln_sign ? 1 : -1;
		}
	}

	for (size_t i = mini; i > 0; i--)
	{
		if (ln_num[i - 1] > r.ln_num[i - 1])
		{
			return ln_sign ? -1 : 1;
		}
		else if (ln_num[i - 1] < r.ln_num[i - 1])
		{
			return ln_sign ? 1 : -1;
		}
	}

	return 0;
}

bool LN::is_null() const
{
	if (ln_nan)
		return false;
	return std::all_of(ln_num.begin(), ln_num.end(), [](LN_TYPE x) { return x == 0; });
}

void LN::div_2()
{
	for (int i = 0; i < ln_num.size(); i++)
	{
		ln_num[i] >>= 1;
		if (i + 1 < ln_num.size())
		{
			LN_TYPE extra = ln_num[i + 1] % 2;
			if (extra == 1)
			{
				ln_num[i] |= extra << (LN_HEX_BLOCKS * 4 - 1);
			}
			else
			{
				ln_num[i] &= ~(extra << (LN_HEX_BLOCKS * 4 - 1));
			}
		}
	}
}

template< typename T >
bool LN::check_is_number(int s_len, T s_num)
{
	bool is_num = true;
	for (int i = 0; i < s_len; i++)
	{
		if ((i != 0 && (s_num[i] == '+' || s_num[i] == '-')) ||
			!(std::isdigit(s_num[i]) || (std::toupper(s_num[i]) >= 'A' && std::toupper(s_num[i]) <= 'F') ||
			  s_num[i] == '+' || s_num[i] == '-'))
		{
			is_num = false;
			break;
		}
	}
	return is_num;
}
