# quick-markov
Cute little C++ Markov chain program.

Compile with `g++ -O3 --std=c++17 markov.cpp -o markov`

The resulting executable reads from standard input. `markov < inputfile`, `someotherprogram | markov`, and running `markov` on its own, typing some input, then ctrl-D on its own line all work.

If your training data is coming from a pipe or file, using `pv filename | markov` or `someotherprogram | pv | markov` can give you a progress bar for the training process.

It treats each sequence of bytes separated by whitespace as a word and each sequence of bytes separated by a newline as an entry. This can be changed by changing the `is_word_boundary` function and giving a third argument to `std::getline` respectively.

By default, it produces one line of output. Passing a number as a command line argument, like `markov 20`, will produce that many lines instead.
