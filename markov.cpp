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

#ifdef NOVEL_OUTPUT
#include <unordered_set>

std::unordered_set<std::string> input_lines;
#endif


static bool is_word_boundary(char c)
{
	// see https://en.cppreference.com/w/cpp/string/byte/isspace
	return std::isspace(static_cast<unsigned char>(c));
}

static markov_model train_model()
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
			// basically, capture the space after the word, if there is one
			// this means that the model will accurately use tabs, newlines, zero-width spaces, etc. in addition to regular old spaces
			// note that newlines won't show up in the output by default because this program assumes a newline terminates a statement
			// this behavior can be changed by passing a third parameter to std::getline
			// if you do use something other than a newline to terminate statements, note that output is, by default, also newline-separated.
			// also note that markov_model relies on this behavior- if you want it to automatically insert spaces, see the comment inside markov_model::generate
			words.emplace_back(word_start, word_end + (word_end != buffer.end()));
			word_start = std::find_if_not(word_end, buffer.cend(), is_word_boundary);
		} 
		model.train(std::make_move_iterator(words.begin()), std::make_move_iterator(words.end()));
		words.clear();
#ifdef NOVEL_OUTPUT
		input_lines.insert(buffer);
#endif
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
		auto output = model.generate();
#ifdef NOVEL_OUTPUT
		while (input_lines.count(output) == 1)
			output = model.generate();
#endif
		std::cout << output << '\n';
	}
}
