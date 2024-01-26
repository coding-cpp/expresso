#include <expresso/core/server.h>
#include <expresso/utils/process.h>

using namespace expresso::core;

void helloWorldHandler(Request &req, Response &res) {
  // response to GET /hello/world
  res.status(StatusCode::OK).send("Hello World!");
  return;
}

int main(int argc, char **argv) {
  expresso::utils::Process process("../.env");
  Server app;
  Router world;

  world.get("/world", helloWorldHandler);
  app.use("/hello", &world);

  app.get("/hello", [](Request &req, Response &res) {
    // response to GET /hello
    res.status(StatusCode::OK).send("Hello!");
    return;
  });

  app.run(std::stoi(process.getEnv("PORT")));

  return EXIT_SUCCESS;
}