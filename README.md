# Wally

Wally is a dynamically typed programming language styled primarily after C and featuring modern programming patterns such as OOP.

You can try it out now [in your browser!](https://cuber01.github.io/wally-web/)

It's also possible to make [games in it.](https://github.com/Cuber01/fenko-wally)

The documentation can be found at [the wiki](https://github.com/Cuber01/Wally/wiki).

# Running

## Build

### Native

```bash
cmake .
make
```

If it fails because of your cmake version being too old, edit this line in `CMakeLists.txt`
```cmake
cmake_minimum_required(VERSION <YOUR CMAKE VERSION>)
```

### Emscripten

```
emcmake cmake . -DEMSCRIPTEN=1
emmake make
```

## Run

```
./Wally
```

See the documentation or `--help` for more info.

# Documentation

See wiki at https://github.com/Cuber01/Wally/wiki.

# Tests

## Running

`test.py` script is configured to run Wally's executable at `<script location>/build/release/Wally`
Make sure that it is in the mentioned location.

Next, run:
```
python3 test.py
```

## Writing

Write a `.wally` file and put it in one of `tests/`s subdirectories.

Add expected output in the following format:

```js
print(1); // Expect: 1

print(2);
print(3);
print(4);

// Expect: 2
// Expect: 3
// Expect: 4
```

To add a new directory, you'll have to edit the script.

# Credits

Robert Nystrom for writing a [great book](http://craftinginterpreters.com/) and a pretty good test.py script.

[Egor Dorichev](https://github.com/egordorichev/lit) for inspiration, support and the adapted test.py script.

[@Cuber01](https://github.com/Cuber01) for writing this thing.

# History and other language projects

This is my seventh full language implementation. The first four were a disaster and most of them are either hidden or lost. 

[This]() is my fifth attempt, it's a single pass AST Tree walker written in C#.

The 6th attempt was Wally before I decided to rewrite it into a double pass compiler. The single pass version is half-working and resides on a legacy branch in the current repo.

[This](https://github.com/Cuber01/brainfuck-interpreter-c) is a brainfuck interpreter I made.


