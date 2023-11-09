#include <expresso/email.h>
#include <expresso/process.h>

int main(int argc, char **argv) {
  Process process("../.env");

  Email gmail("smtp.gmail.com", 587);
  gmail.login(process.getEnv("USERNAME"), process.getEnv("PASSWORD"));
  gmail.send("jadit19@gmail.com", "Test", "Hello World!");

  return EXIT_SUCCESS;
}