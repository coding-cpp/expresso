// Include files
#include <expresso/core/server.h>
#include <expresso/middleware/cookie_parser.h>
#include <expresso/middleware/cors.h>
#include <expresso/middleware/static_serve.h>
#include <expresso/utils/process.h>

// Personally, I don't encourange using namespaces, but, I left it here just so
// that the code could be more readable ¯\_(ツ)_/¯
using namespace expresso::core;
using namespace expresso::middleware;
using namespace expresso::utils;

// Global variable, just for fun :)
int port;

// Handler to handle the `GET` request on `/hello/world`
void helloWorldHandler(Request &req, Response &res) {
  json::object resp;
  resp["framework"] = "Expresso";
  resp["repository"] = "https://github.com/coding-cpp/expresso";
  resp["language"] = "C++";
  resp["creator"] = "Adit Jain";

  json::object submodules;
  submodules.push_back(json::object());
  submodules[0]["repository"] = "https://github.com/coding-cpp/logger";
  submodules[0]["work"] = "Logging library for C++";
  submodules[0]["location"] = "lib/logger";
  submodules[0]["name"] = "Logger";

  json::object nexusSubmodule;
  nexusSubmodule["repository"] = "https://github.com/coding-cpp/nexus";
  nexusSubmodule["work"] = "Thread pool library for C++";
  submodules.push_back(nexusSubmodule);
  submodules[1]["location"] = "lib/nexus";
  submodules[1]["name"] = "Nexus";

  submodules.push_back(json::object());
  resp["submodules"] = submodules;

  resp["submodules"][2]["repository"] = "https://github.com/coding-cpp/json";
  resp["submodules"][2]["work"] = "JSON library for C++";
  resp["submodules"][2]["location"] = "lib/json";
  resp["submodules"][2]["name"] = "JSON";

  res.json(resp).status(200);
  return;
}

int main(int argc, char **argv) {
  // `Process` class to load in the environment variables
  Process process("../.env");
  port = std::stoi(process.getEnv("PORT"));

  // Running a server with max 10 connections and 4 threads
  Server app(10000, 100);

  // Router, for routing requests starting with /hello
  Router world;

  // Cors middleware
  Cors cors;
  cors.allowOrigin("*.app.localhost");
  cors.allowCredentials(true);
  // app.use(&cors);

  // CookieParser middleware
  CookieParser cookieParser;
  app.use(&cookieParser);

  // StaticServe middleware
  StaticServe staticServe("../assets");
  app.use(&staticServe);

  // Routing demo
  world.get("/world", helloWorldHandler);
  app.use("/hello", &world);

  // Direct usage demo
  // response to GET request on /hello
  app.get("/hello", [](Request &req, Response &res) {
    res.status(StatusCode::OK).send("Hello!");
    return;
  });

  // Example sending file in response to GET request on /expresso
  app.get("/expresso", [](Request &req, Response &res) {
    res.sendFile("../assets/coffee.png");
    return;
  });

  // Listening on port with optional callback function
  app.listen(port, []() {
    logger::success("Listening on port " + std::to_string(port));
  });

  return EXIT_SUCCESS;
}