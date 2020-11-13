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
#include <iterator>

#ifdef MARKOV_PARALLEL
#include <execution>
#include <functional>
#define MARKOV_PARALLEL_POLICY std::execution::par_unseq,
#else
#define MARKOV_PARALLEL_POLICY
#endif

#if __cpp_if_constexpr == 201606
#define IF_CONSTEXPR constexpr
#else
#define IF_CONSTEXPR
#endif

class markov_model
{
	using word_index_t = size_t;

	public:

		template <typename stringlike_iterator>
		void train(stringlike_iterator begin, stringlike_iterator end)
		{
			if (begin == end) { return; }
			const auto word_indexes = indexify(begin, end);

			// It's not worth parallelizing this because:
			// - word_indexes is unlikely to be long enough to get any speedup 
			// - if the same word is in the sentence twice, that creates a race condition on the following_weights for that index.
			const auto last = std::accumulate(std::next(word_indexes.cbegin()), word_indexes.cend(), word_indexes.front(), [this](word_index_t curr, word_index_t next)
			{
				add_or_increment_index(following_weights[curr], next);
				return next;
			});
			add_or_increment_index(following_weights[last], end_output);
		}

		std::string generate() const
		{
			auto next_index = random_sample(starting_words);

			std::vector<word_index_t> indexes;

			while (next_index != end_output)
			{
				indexes.push_back(next_index);
				next_index = random_sample(following_weights[next_index]);
			}

			return std::accumulate(std::next(indexes.cbegin()), indexes.cend(), known_words[indexes.front()], [this](std::string& acc, word_index_t curr) { return acc.append(1, ' ').append(known_words[curr]); });
		}

	private:

		using count_t = uint_fast32_t;
		struct word_weight
		{
			word_index_t word_index;
			count_t count;
		};

		const word_index_t end_output = -1;

		std::vector<std::string> known_words;
		std::vector<word_weight> starting_words;
		std::vector<std::vector<word_weight>> following_weights;

		// this is mutable so it can be used from inside random_sample and generate.
		// if you want generate to be thread safe, put a mutex around the roll line below
		// or give each thread its own random number generator and pass it into generate or random_sample.
		mutable std::minstd_rand rand{std::random_device{}()};

		word_index_t random_sample(const std::vector<word_weight>& vec) const
		{
			if (vec.size() == 1) { return vec[0].word_index; }

#ifdef MARKOV_PARALLEL
			const auto total_weight = std::transform_reduce(MARKOV_PARALLEL_POLICY vec.cbegin(), vec.cend(), (count_t)0, std::plus<>{}, [](const word_weight& weight) { return weight.count; });
#else
			const auto total_weight = std::accumulate(vec.cbegin(), vec.cend(), (count_t)0, [](const auto acc, const auto& current) { return acc + current.count; });
#endif

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
			const auto found = std::find_if(MARKOV_PARALLEL_POLICY follow_weight.begin(), follow_weight.end(), [word_index](const auto& weight) {
				return weight.word_index == word_index;
			});

			if (found != follow_weight.end())
				found->count++;
			else
				follow_weight.push_back(word_weight{word_index, 1});
		}

		template <typename stringlike_iterator>
		std::vector<word_index_t> indexify(stringlike_iterator begin, stringlike_iterator end)
		{
			std::vector<word_index_t> word_indexes;
			using category = typename std::iterator_traits<stringlike_iterator>::iterator_category;
			if IF_CONSTEXPR (std::is_convertible<category, std::random_access_iterator_tag>::value)
			{
				// only bother reserving if we can compute std::distance in constant time
				word_indexes.reserve(std::distance(begin, end));
			}

			// Paralellizing this one looks like it gives worse performance.
			// I suspect it's because words is rarely very long, and there's contention on the reader-writer lock you have to put on known_words.
			std::transform(begin, end, std::back_inserter(word_indexes),
				[this](auto&& word) {
					auto word_index = index_of(word);
					if (word_index == -1)
					{
						known_words.emplace_back(std::forward<typename stringlike_iterator::value_type>(word));
						word_index = known_words.size() - 1;
						following_weights.push_back({});
					}
					return word_index;
				});

			add_or_increment_index(starting_words, word_indexes.front());
			return word_indexes;
		}

		template <typename stringlike>
		word_index_t index_of(const stringlike& word) const
		{
			const auto word_it = std::find(MARKOV_PARALLEL_POLICY known_words.cbegin(), known_words.cend(), word);
			return word_it == known_words.cend() ? -1 : std::distance(known_words.cbegin(), word_it);
		}
};

#endif
