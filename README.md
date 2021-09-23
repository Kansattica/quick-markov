# quick-markov
Cute little C++ Markov chain program.

## Using the provided runner
Compile with `g++ -O3 markov.cpp -o markov`

The resulting executable reads from standard input. `markov < inputfile`, `someotherprogram | markov`, and running `markov` on its own, typing some input, then ctrl-D on its own line all work.

If your training data is coming from a pipe or file, using `pv filename | markov` or `someotherprogram | pv | markov` can give you a progress bar for the training process.

It treats each sequence of bytes separated by whitespace as a word and each sequence of bytes separated by a newline as an entry. These can be changed by changing the `is_word_boundary` function and giving a third argument to `std::getline` respectively. The program skips over duplicate whitespace, but otherwise does not clean the input data at all.

By default, it produces one line of output. Passing a number as a command line argument, like `markov 20`, will produce that many lines instead.

You can define `MARKOV_TIMING` (compile with `-DMARKOV_TIMING` or add `#define MARKOV_TIMING` to `markov.cpp`) to have the program calculate and print how fast it's processing the training data, but note that this slows down the training step quite a bit. You get more accurate measurements by having something out-of-band, like `pv` or `/usr/bin/time` do the timing. 

Similarly, you can define `NOVEL_OUTPUT` to ensure that the runner doesn't produce output that's in the input corpus. It does this by maintaining a std::set of all the input lines and generating output again if anything matches. This slows down training (because it has to insert each line into a data structure), increases memory usage (because it has to maintain a copy of the input corpus in memory), and can slow down output on small or highly repetitive input corpuses, but it also guarantees that the program is generating novel output.

## Using `markov_model`
You can also use `markov_model.hpp` in your own projects. It's a class, `markov_model`, that exposes two functions: `train` and `generate`. 

`train` takes a begin and end iterator pointing to `stringlike`s and adds the sequence to the model. If you would like `train` to `std::move` from the strings to conserve time and memory, simply wrap `begin` and `end` in `std::make_move_iterator`, as seen in `markov.cpp`. Each call to `train` is treated as one message. This is important because the first word is marked as a reasonable word to start a generated message on, and the last word is treated as a reasonable word to end a message on. A `stringlike` is anything that can be used to construct a `std::string` and compared to a `std::string` using `operator==`. Notably, this includes `std::string_view`. 

`generate` uses the trained model to produce a `std::string`. Note that you can freely intermix calls to `train` and `generate`- there's no intermediate step to "finalize" a model. If you'd like to prevent your model from being trained after a certain point, a `const markov_model` can `generate`, but not `train`. 

You can also define `MARKOV_PARALLEL` to enable experimental parallel processing support when training `markov_model` and generating output. This is useful for larger corpuses (over 2000 unique words or so, always benchmark and compare to be sure). This requires a compiler and library with C++ 17 support, specifically the `<execution>` header. You might have to pass the `-std=c++17` or equivalent flag to your compiler.
