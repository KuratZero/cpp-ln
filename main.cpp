//
// Created by kirat on 19.09.23.
//

#include "LN.h"
#include "return_codes.h"

#include <cstring>
#include <fstream>
#include <iostream>
#include <set>
#include <stack>
#include <string>

int main(int argc, char *argv[])
{
	if (argc != 4)
	{
		std::cerr << "The count of parameters is incorrect(" << argc - 1 << " of 3), must be (... input output order)\n";
		return ERROR_PARAMETER_INVALID;
	}

	bool order;
	if (strcmp(argv[3], "direct") == 0)
	{
		order = true;
	}
	else if (strcmp(argv[3], "inverse") == 0)
	{
		order = false;
	}
	else
	{
		std::cerr << "Unsupported order '" << argv[3] << "'.\n";
		return ERROR_PARAMETER_INVALID;
	}

	std::ifstream in(argv[1]);
	if (!in.is_open())
	{
		std::cerr << "Input file '" << argv[1] << "' can't open for read.\n";
		return ERROR_CANNOT_OPEN_FILE;
	}
	std::vector< std::string > in_str;
	while (!in.eof())
	{
		std::string tmp;
		std::getline(in, tmp);
		if (tmp.empty())
			continue;
		try
		{
			in_str.push_back(tmp);
		} catch (std::bad_alloc &alloc)
		{
			in.close();
			std::cerr << "Error with allocate memory while read : " << alloc.what() << '\n';
			return ERROR_OUT_OF_MEMORY;
		} catch (std::exception &exception)
		{
			in.close();
			std::cerr << "Unknown error while computing : " << exception.what() << '\n';
			return ERROR_UNKNOWN;
		}
	}
	in.close();

	std::set< std::string > operations = { "+", "-", "*", "/", "%", "<", "<=", ">", ">=", "==", "!=" };

	std::stack< LN > st;

	for (auto &i : in_str)
	{
		try
		{
			if (i == "~" || i == "_")
			{
				LN x = st.top();
				st.pop();
				st.push((i == "~") ? (~x) : (-x));
				continue;
			}

			LN l, r;
			if (operations.count(i))
			{
				if (order)
				{
					l = st.top();
				}
				else
				{
					r = st.top();
				}
				st.pop();
				if (order)
				{
					r = st.top();
				}
				else
				{
					l = st.top();
				}
				st.pop();
			}

			if (i == "+")
				st.push(l + r);
			else if (i == "-")
				st.push(l - r);
			else if (i == "*")
				st.push(l * r);
			else if (i == "/")
				st.push(l / r);
			else if (i == "%")
				st.push(l % r);
			else if (i == "<")
				st.emplace(l < r);
			else if (i == "<=")
				st.emplace(l <= r);
			else if (i == ">")
				st.emplace(l > r);
			else if (i == ">=")
				st.emplace(l >= r);
			else if (i == "==")
				st.emplace(l == r);
			else if (i == "!=")
				st.emplace(l != r);
			else
				st.emplace(i);
		} catch (std::bad_alloc &alloc)
		{
			std::cerr << "Error with allocate memory while computing : " << alloc.what() << '\n';
			return ERROR_OUT_OF_MEMORY;
		} catch (std::exception &exception)
		{
			std::cerr << "Unknown error while computing : " << exception.what() << '\n';
			return ERROR_UNKNOWN;
		}
	}

	std::ofstream out(argv[2]);
	if (!out.is_open())
	{
		std::cerr << "Input file '" << argv[2] << "' can't open for write.\n";
		return ERROR_CANNOT_OPEN_FILE;
	}

	while (!st.empty())
	{
		out << st.top().to_hex_string() << '\n';
		st.pop();
	}

	out.close();
	return SUCCESS;
}
