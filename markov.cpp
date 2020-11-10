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
