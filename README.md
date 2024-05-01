# Expresso ☕️

A C++ backend framework designed for web applications. A few features include:

- Simple and easy to use API ✨
- Built-in HTTP server 🌐
- Out of the box multi-threading support 🧵
- Expandable custom middleware 🧩

It is pretty straightforward and if you are familiar with the Express.js framework, you will feel right at home, as the API is very similar, which is my goal? 🎯

## Installation 🚀

0. Make sure you have the gnu g++ compiler installed on your system. You can install it by running the following command:

```bash
sudo apt-get install g++ build-essential
```

1. Clone the repository:

```bash
git clone --recurse-submodules git@github.com:coding-cpp/expresso.git
```

2. Build the project:

```bash
cd expresso
mkdir build && cd build
cmake ..
make
```

3. Run the server:

```bash
./expresso
```

## Example

You can find an example of how to use the framework in the [`example`](./example/) directory.

## Bugs and Issues

If you encounter any bugs or issues, feel free to open an issue on the repository. This is a work in progress and since I am the only one working on it, that too in my spare time, I might not have the time to fix it right away, but I will try my best to get to it as soon as possible. 🐛🔧
