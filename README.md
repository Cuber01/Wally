# Wally

TBA

# Running

## Build

```bash
cmake .
make
```

If it fails because of your cmake version being too old, edit this line in `CMakeLists.txt`
```cmake
cmake_minimum_required(VERSION <YOUR CMAKE VERSION>)
```

## Run

```
./Wally
```

See the documentation or `--help` for more info.

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

