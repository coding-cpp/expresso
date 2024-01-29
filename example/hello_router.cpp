#include <expresso/core/server.h>
#include <expresso/middleware/cookie_parser.h>
#include <expresso/middleware/cors.h>
#include <expresso/utils/process.h>

using namespace expresso::core;
using namespace expresso::middleware;
using namespace expresso::utils;

int port;

void helloWorldHandler(Request &req, Response &res) {
  // response to GET /hello/world
  res.status(StatusCode::OK).send("Hello World!");
  return;
}

int main(int argc, char **argv) {
  Process process("../.env");
  port = std::stoi(process.getEnv("PORT"));

  Server app(10);
  Router world;

  Cors cors;
  CookieParser cookieParser;

  cors.allowOrigin("*.app.localhost");
  cors.allowCredentials(true);
  app.use(&cors);
  app.use(&cookieParser);

  world.get("/world", helloWorldHandler);
  app.use("/hello", &world);

  app.get("/hello",
          [](Request &req,
             Response &res) { // Capture 'port' in the lambda capture list
            // response to GET /hello
            res.status(StatusCode::OK).send("Hello!");
            return;
          });

  app.listen(port, []() {
    print::success("Listening on port " + std::to_string(port));
  });

  return EXIT_SUCCESS;
}