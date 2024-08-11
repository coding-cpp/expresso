// Include necessary header files
#include <expresso/core/server.h>
#include <expresso/middleware/cookie_parser.h>
#include <expresso/middleware/cors.h>
#include <expresso/middleware/static_serve.h>

#include <brewtils/env.h>

// Personally, I don't encourange using namespaces, but, I left it here just so
// that the code could be more readable ¯\_(ツ)_/¯
using namespace expresso::core;
using namespace expresso::middleware;

// Global variable, just for fun :)
int port;

// About handler
void about(Request &req, Response &res) {
  json::object response;
  response["framework"] = "Expresso";
  response["repository"] = "https://github.com/coding-cpp/expresso";
  response["language"] = "C++";
  response["creator"]["name"] = "Adit Jain";
  response["creator"]["github"] = "https://github.com/jadit19";
  response["submodules"].resize(5);

  response["submodules"][0]["repository"] =
      "https://github.com/coding-cpp/logger";
  response["submodules"][0]["work"] = "Logging library for C++";
  response["submodules"][0]["location"] = "lib/logger";
  response["submodules"][0]["name"] = "Logger";

  response["submodules"][1]["repository"] =
      "https://github.com/coding-cpp/nexus";
  response["submodules"][1]["work"] = "Thread pool library for C++";
  response["submodules"][1]["location"] = "lib/nexus";
  response["submodules"][1]["name"] = "Nexus";

  response["submodules"][2]["repository"] =
      "https://github.com/coding-cpp/json";
  response["submodules"][2]["work"] = "JSON library for C++";
  response["submodules"][2]["location"] = "lib/json";
  response["submodules"][2]["name"] = "JSON";

  response["submodules"][3]["repository"] =
      "https://github.com/coding-cpp/brewtils";
  response["submodules"][3]["work"] = "Utilities library for C++";
  response["submodules"][3]["location"] = "lib/brewtils";
  response["submodules"][3]["name"] = "Brewtils";

  response["submodules"][4]["repository"] =
      "https://github.com/coding-cpp/zippuccino";
  response["submodules"][4]["work"] = "Zipping library for C++";
  response["submodules"][4]["location"] = "lib/zippuccino";
  response["submodules"][4]["name"] = "Zippuccino";

  res.status(STATUS_CODE::OK).json(response).end();
  return;
}

int main(int argc, char **argv) {
  brewtils::env::init("../.env");
  port = std::stoi(brewtils::env::get("PORT", "8000"));

  Server app(10, 4);

  // CORS middleware
  Cors cors;
  cors.allowOrigin("*");
  cors.allowCredentials(true);
  app.use(&cors);

  // Cookie Parser
  CookieParser cookieParser;
  app.use(&cookieParser);

  // Static serve middleware
  StaticServe staticServe("../assets");
  app.use(&staticServe);

  // Route handling like normal
  app.get("/health", [](Request &req, Response &res) {
    res.status(STATUS_CODE::OK).send("Running healthy");
  });

  // Creating and using a router
  Router router;
  router.get("/", about);
  app.use("/about", &router);

  // Listing directories
  StaticServe pictureServe("../assets/github", true);
  Router pictureRouter;
  pictureRouter.use(&pictureServe);
  app.use("/pictures", &pictureRouter);

  // Sending multiple files as single zip
  app.get("/download", [](Request &req, Response &res) {
    // You can put folders too, it will zip all the files in the folder
    std::set<std::string> files = {"../assets"};
    res.sendFiles(files);
  });

  // Starting the server
  app.listen(port, []() {
    logger::success("Server is running on port " + std::to_string(port));
  });

  return 0;
}