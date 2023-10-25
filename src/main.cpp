#include <expresso/server.h>

void helloHandler(int clientSocket) {
  std::string response = "Hello, World!";

  std::string header = "HTTP/1.1 200 OK\r\n";
  header += "Content-Type: text/plain\r\n";
  header += "Content-Length: " + std::to_string(response.length()) + "\r\n";
  header += "\r\n";

  send(clientSocket, header.c_str(), header.length(), 0);
  send(clientSocket, response.c_str(), response.length(), 0);

  close(clientSocket);

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