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

#ifndef MARKOV_MODEL_HPP
#define MARKOV_MODEL_HPP

#include <vector>
#include <random>
#include <string>
#include <algorithm>
#include <numeric>

class markov_model
{
	using word_index_t = size_t;

	public:

		void train(std::vector<std::string>& words)
		{
			if (words.empty()) { return; }
			const auto word_indexes = indexify(words);
			for (size_t i = 0; i < word_indexes.size() - 1; i++)
			{
				add_or_increment_index(following_weights[word_indexes[i]], word_indexes[i + 1]);
			}

			add_or_increment_index(following_weights[word_indexes.back()], end_output);
		}

		std::string generate()
		{
			auto next_index = random_sample(starting_words);

			std::vector<word_index_t> indexes;

			while (next_index != end_output)
			{
				indexes.push_back(next_index);
				const auto& next_words = following_weights[next_index];
				if (next_words.empty())
					break;
				next_index = random_sample(next_words);
			}

			return std::accumulate(std::next(indexes.begin()), indexes.end(), known_words[indexes.front()], [this](std::string& acc, word_index_t curr) { return acc.append(1, ' ').append(known_words[curr]); });
		}

	private:

		struct word_weight
		{
			word_index_t word_index;
			int count;
		};

		const word_index_t end_output = -1;

		std::vector<std::string> known_words;
		std::vector<word_weight> starting_words;
		std::vector<std::vector<word_weight>> following_weights;

		std::mt19937_64 rand{std::random_device{}()};

		word_index_t random_sample(const std::vector<word_weight>& vec)
		{
			if (vec.size() == 1) { return vec[0].word_index; }

			const auto total_weight = std::accumulate(vec.begin(), vec.end(), 0, [](const auto acc, const auto& current) { return acc + current.count; });

			std::uniform_int_distribution<> range(1, total_weight);

			auto roll = range(rand);

			for (const auto& word : vec)
			{
				roll = roll - word.count;
				if (roll <= 0)
					return word.word_index;
			}

			return vec.back().word_index;
		}

		void add_or_increment_index(std::vector<word_weight>& follow_weight, word_index_t word_index)
		{
			const auto found = std::find_if(follow_weight.begin(), follow_weight.end(), [word_index](const auto& weight)
					{
					return weight.word_index == word_index;
					});

			if (found != follow_weight.end())
				found->count++;
			else
				follow_weight.push_back(word_weight{word_index, 1});
		}

		std::vector<word_index_t> indexify(std::vector<std::string>& words)
		{
			std::vector<word_index_t> word_indexes;
			word_indexes.reserve(words.size());

			bool first = true;
			for (auto&& word : words)
			{
				auto word_index = index_of(word);
				if (word_index == -1)
				{
					known_words.push_back(std::move(word));
					word_index = known_words.size() - 1;
					following_weights.push_back({});
				}
				word_indexes.push_back(word_index);
				if (first)
				{
					add_or_increment_index(starting_words, word_index);
					first = false;
				}
			}
			words.clear();

			return word_indexes;
		}

		word_index_t index_of(const std::string& word)
		{
			for (word_index_t i = 0; i < known_words.size(); i++)
			{
				if (known_words[i] == word)
					return i;
			}
			return -1;
		}

};

#endif
