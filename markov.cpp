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

#ifdef MARKOV_TIMING
#include <chrono>
#endif

#include "markov_model.hpp"

bool is_word_boundary(char c)
{
	// see https://en.cppreference.com/w/cpp/string/byte/isspace
	return std::isspace(static_cast<unsigned char>(c));
}

markov_model train_model()
{
	std::vector<std::string> words;
	markov_model model;
#ifdef MARKOV_TIMING
	size_t bytes_crunched = 0;
	const auto start = std::chrono::high_resolution_clock::now();
#endif
	for (std::string buffer; std::getline(std::cin, buffer);)
	{
		auto word_start = std::find_if_not(buffer.cbegin(), buffer.cend(), is_word_boundary);
		while (word_start != buffer.end())
		{
			const auto word_end = std::find_if(word_start, buffer.cend(), is_word_boundary);
			words.emplace_back(word_start, word_end);
			word_start = std::find_if_not(word_end, buffer.cend(), is_word_boundary);
		} 
		model.train(words);
#ifdef MARKOV_TIMING
		bytes_crunched += buffer.size();
		const auto end = std::chrono::high_resolution_clock::now();
		const std::chrono::duration<double, std::milli> elapsed = end - start;
		std::cerr << "Crunched " << bytes_crunched/1000 << " kb in " << elapsed.count() << " ms. " << bytes_crunched/elapsed.count() << " kb/sec.   \r";
#endif
	}

#ifdef MARKOV_TIMING
	std::cerr << '\n'; // leave the timing data on the screen
#endif

	return model;
}

int main(int argc, char** argv)
{
	const auto model = train_model();

	const int to_generate = argc > 1 ? std::atoi(argv[1]) : 1;

	for (int i = 0; i < to_generate; i++)
	{
		std::cout << model.generate() << '\n';
	}
}
