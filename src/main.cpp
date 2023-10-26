#include <iostream>
#include <stdexcept>

#include "app.hpp"

int main() {

	auto app = engine::App {};
	try {
		app.run();
	} catch (const std::exception &e) {
		std::cerr << e.what() << "\n";
		return -1;
	}
	return 0;
}
