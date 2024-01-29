#include <expresso/core/server.h>
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
  Server app(10);
  Router world;
  Cors cors;

  port = std::stoi(process.getEnv("PORT"));

  cors.allowOrigin("*.app.localhost");
  cors.allowCredentials(true);
  app.use(&cors);

  world.get("/world", helloWorldHandler);
  app.use("/hello", &world);

  app.get("/hello",
          [](Request &req,
             Response &res) { // Capture 'port' in the lambda capture list
            // response to GET /hello
            res.status(StatusCode::OK).send("Hello!");
            return;
          });

  app.run(port, []() {
    print::success("Listening on port " + std::to_string(port));
  });

  return EXIT_SUCCESS;
}