#include <iostream>
#include <memory>

using namespace std;

main() {
  auto pi = make_unique<int>(5);
  cout << "Hello, world " << (*pi) << endl;
}
