/*
	quick-markov, a cute little markov chain program
    Copyright (C) 2020 Grace Lovelace

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as published
    by the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.


	compile with:
	g++ -O3 --std=c++17 markov.cpp -o markov
 */


#include <iostream>
#include <string>
#include <cctype>
#include <algorithm>
#include <cstdlib>

#include "markov_model.hpp"

constexpr bool is_word_boundary(char c)
{
	// see https://en.cppreference.com/w/cpp/string/byte/isspace
	return std::isspace(static_cast<unsigned char>(c));
}

int main(int argc, char** argv)
{
	std::vector<std::string> words;
	markov_model model;
	for (std::string buffer; std::getline(std::cin, buffer);)
	{
		auto word_start = std::find_if_not(buffer.begin(), buffer.end(), is_word_boundary);
		if (word_start == buffer.end()) { continue; }
		do
		{
			const auto word_end = std::find_if(word_start, buffer.end(), is_word_boundary);
			words.emplace_back(word_start, word_end);
			word_start = std::find_if_not(word_end, buffer.end(), is_word_boundary);
		} while(word_start != buffer.end());
		model.train(words);
	}

	const int to_generate = argc > 1 ? std::atoi(argv[1]) : 1;

	for (int i = 0; i < to_generate; i++)
	{
		std::cout << model.generate() << '\n';
	}
}
