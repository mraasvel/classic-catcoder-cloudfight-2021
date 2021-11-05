#include <vector>
#include <string>

/*
LOGIC:
	1. SKIP leading DELIM
	2. FIND next DELIM
	3. SUBSTR until that DELIM
	4. SKIP next DELIMS
*/

/*
create vector containing substrings from input split by delim
*/
std::vector<std::string> splitString(const std::string& input, char delim)
{
	std::vector<std::string> lines;

	size_t i = input.find_first_not_of(delim);
	while (i < input.length()) {
		size_t end = input.find_first_of(delim, i);
		lines.push_back(input.substr(i, end - i));
		i = input.find_first_not_of(delim, end);
	}
	return lines;
}