#include <expresso/email.h>

Email::Email(std::string server, int port) {
  this->server = server;
  this->port = port;

  this->socket = sys::socket_wrapper(AF_INET, SOCK_STREAM, 0);
  if (this->socket < 0) {
    std::cout << "[ERROR] Email socket not created!" << std::endl;
    return;
  }

  memset(&this->hints, 0, sizeof(this->hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  if (getaddrinfo(this->server.c_str(), std::to_string(this->port).c_str(),
                  &this->hints, &this->result) != 0) {
    std::cout << "[ERROR] Could not get address info for " + this->server
              << std::endl;
    return;
  }

  this->address = (struct sockaddr_in *)this->result->ai_addr;

  return;
}

Email::~Email() { return; }

void Email::login(std::string username, std::string password) {
  this->username = username;
  this->password = password;
  this->encodedUsername = base64_encode(this->username);
  this->encodedPassword = base64_encode(this->password);

  return;
}

bool Email::send(std::string recipient, std::string subject,
                 std::string message) {
  if (connect(this->socket, (struct sockaddr *)this->address,
              sizeof(*this->address)) < 0) {
    std::cerr << "[ERROR] Could not connect to: " << this->server << std::endl;
    return false;
  }

  bool response = false;
  try {
    char buffer[8192];

    // Read initial response
    std::cout << "[RUNNING]: Greeting" << std::endl
              << "----------------------------------" << std::endl;
    recv(this->socket, buffer, sizeof(buffer), 0);
    std::cout << buffer << std::endl;

    // Send HELO command
    std::cout << "[RUNNING]: EHLO Adit" << std::endl
              << "----------------------------------" << std::endl;
    sys::send_wrapper(this->socket, "EHLO Adit\r\n", strlen("EHLO Adit\r\n"),
                      0);
    recv(this->socket, buffer, sizeof(buffer), 0);
    std::cout << buffer << std::endl << std::endl;

    // Send STARTTLS command
    std::cout << "[RUNNING]: STARTTLS" << std::endl
              << "----------------------------------" << std::endl;
    sys::send_wrapper(this->socket, "STARTTLS\r\n", strlen("STARTTLS\r\n"), 0);
    recv(this->socket, buffer, sizeof(buffer), 0);
    std::cout << buffer << std::endl << std::endl;

    // Send AUTH command
    std::cout << "[RUNNING]: AUTH LOGIN" << std::endl
              << "----------------------------------" << std::endl;
    sys::send_wrapper(this->socket, "AUTH LOGIN\r\n", strlen("AUTH LOGIN\r\n"),
                      0);
    recv(this->socket, buffer, sizeof(buffer), 0);
    std::cout << buffer << std::endl << std::endl;

    // Send username
    std::cout << "[RUNNING]: " << this->encodedUsername << std::endl
              << "----------------------------------" << std::endl;
    sys::send_wrapper(this->socket, (this->encodedUsername + "\r\n").c_str(),
                      strlen((this->encodedUsername + "\r\n").c_str()), 0);
    recv(this->socket, buffer, sizeof(buffer), 0);
    std::cout << buffer << std::endl << std::endl;

    // Send password
    std::cout << "[RUNNING]: " << this->encodedPassword << std::endl
              << "----------------------------------" << std::endl;
    sys::send_wrapper(this->socket, (this->encodedPassword + "\r\n").c_str(),
                      strlen((this->encodedPassword + "\r\n").c_str()), 0);
    recv(this->socket, buffer, sizeof(buffer), 0);
    std::cout << buffer << std::endl;

    response = true;
  } catch (const std::exception &e) {
    std::cerr << "[ERROR] " << e.what() << std::endl;
  }

  close(this->socket);

  return response;
}

void Email::setSignature(std::string signature) {
  this->signature = signature;

  return;
}