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
   g++ -O3 markov.cpp -o markov
*/

#include <iostream>
#include <string>
#include <cctype>
#include <algorithm>
#include <cstdlib>

#include "markov_model.hpp"

const char word_boundary = ' ';

int main(int argc, char** argv)
{
	std::vector<std::string> words;
	markov_model model;
	for (std::string buffer; std::getline(std::cin, buffer);)
	{
		auto word_start = buffer.find_first_not_of(word_boundary);
		while (word_start != std::string::npos)
		{
			const auto word_end = buffer.find(word_boundary, word_start);
			words.emplace_back(buffer, word_start, word_end - word_start);
			if (word_end == std::string::npos) { break; }
			word_start = buffer.find_first_not_of(word_boundary, word_end);
		} 
		model.train(words);
	}

	const int to_generate = argc > 1 ? std::atoi(argv[1]) : 1;

	for (int i = 0; i < to_generate; i++)
	{
		std::cout << model.generate() << '\n';
	}
}
