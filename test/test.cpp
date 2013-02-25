#include <darc/zeroconf/client.hpp>

void callback(const std::string& host, int port)
{
  std::cout << "#callback" << host << port << std::endl;
}

int main(int argc, char** argv)
{

  darc::ID id = darc::ID::create();
  darc::zeroconf::client p(&callback);
  p.add_service(id, 987);
  p.run();
};
