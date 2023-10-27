#include <expresso/process.h>
#include <expresso/server.h>

void helloHandler(Request &request, Response &response) {
  std::string _response = "Hello, World!";
  response.status(StatusCode::OK).json(_response);
  return;
}

int main(int argc, char **argv) {
  Process process("/home/adit/github/expresso/.env");
  Server app;
  Router helloRouter;

  helloRouter.setBasePath("/hello");
  helloRouter.get("/", &helloHandler);
  app.use(&helloRouter);

  int PORT =
      int(process.getEnv("PORT").empty() ? 8000
                                         : std::stoi(process.getEnv("PORT")));
  app.run(PORT);

  return EXIT_SUCCESS;
}