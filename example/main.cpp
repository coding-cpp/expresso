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
  res.status(StatusCode::OK).send("Hello World!");
  return;
}

int main(int argc, char **argv) {
  // `Process` class to load in the environment variables
  Process process("../.env");
  port = std::stoi(process.getEnv("PORT"));

  // Running a server with max 10 connections and 4 threads
  Server app(10, 4);

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