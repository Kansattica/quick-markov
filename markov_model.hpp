#include <vector>
#include <random>
#include <string>
#include <algorithm>
#include <numeric>

class markov_model
{
	public:
		void train(std::vector<std::string>& words)
		{
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

			std::vector<size_t> indexes;

			while (next_index != end_output)
			{
				indexes.push_back(next_index);
				const auto& next_words = following_weights[next_index];
				if (next_words.empty())
					break;
				next_index = random_sample(next_words);
			}

			return std::accumulate(std::next(indexes.begin()), indexes.end(), known_words[indexes.front()], [this](std::string& acc, size_t curr) { return acc.append(1, ' ').append(known_words[curr]); });
		}

	private:

	struct word_weight
	{
		size_t word_index;
		int count;
	};

	const size_t end_output = -1;

	std::vector<std::string> known_words;
	std::vector<word_weight> starting_words;
	std::vector<std::vector<word_weight>> following_weights;

	std::mt19937_64 rand{std::random_device{}()};

	size_t random_sample(const std::vector<word_weight>& vec)
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

	void add_or_increment_index(std::vector<word_weight>& follow_weight, size_t word_index)
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

	std::vector<size_t> indexify(std::vector<std::string>& words)
	{
		std::vector<size_t> word_indexes;
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

	size_t index_of(const std::string& word)
	{
		for (size_t i = 0; i < known_words.size(); i++)
		{
			if (known_words[i] == word)
				return i;
		}
		return -1;
	}

};
