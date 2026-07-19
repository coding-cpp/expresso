// Mirrors the payload built by `about()` in example/main.cpp so every
// framework serves an identical /about response.

const submodule = (name, repository, work, location) => ({
  name,
  repository,
  work,
  location,
});

export const aboutPayload = {
  framework: "Expresso",
  repository: "https://github.com/coding-cpp/expresso",
  language: "C++",
  creator: {
    name: "Adit Jain",
    github: "https://github.com/jadit19",
  },
  submodules: [
    submodule(
      "Logger",
      "https://github.com/coding-cpp/logger",
      "Logging library for C++",
      "lib/logger",
    ),
    submodule(
      "Nexus",
      "https://github.com/coding-cpp/nexus",
      "Thread pool library for C++",
      "lib/nexus",
    ),
    submodule(
      "JSON",
      "https://github.com/coding-cpp/json",
      "JSON library for C++",
      "lib/json",
    ),
    submodule(
      "Brewtils",
      "https://github.com/coding-cpp/brewtils",
      "Utilities library for C++",
      "lib/brewtils",
    ),
    submodule(
      "Zippuccino",
      "https://github.com/coding-cpp/zippuccino",
      "Zipping library for C++",
      "lib/zippuccino",
    ),
    submodule(
      "Mochios",
      "https://github.com/coding-cpp/mochios",
      "HTTP request library for C++",
      "lib/mochios",
    ),
  ],
};
