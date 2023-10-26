#include <expresso/server.h>

void helloHandler(Request &request, Response &response) {
  std::string _response = "Hello, World!";
  response.status(200).json(_response);
  return;
}

int main(int argc, char **argv) {

  Server app;

  Router hello;
  hello.setBasePath("/hello");
  hello.get("/", &helloHandler);
  app.use(&hello);

  app.run(8000);

  return EXIT_SUCCESS;
}