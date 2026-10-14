#include "app/client_app.h"

#include <cstdint>
#include <cstring>
#include <iostream>

namespace {

constexpr std::uint16_t kPort = 54545;

void printUsage(const char* argv0) {
	std::cout << "Usage: " << argv0 << "\n"
			  << "  open SFML window, Move ↔ WorldState loop on port " << kPort << "\n";
}

}  // namespace

int main(int argc, char** argv) {
	for (int i = 1; i < argc; ++i) {
		if (std::strcmp(argv[i], "--help") == 0 || std::strcmp(argv[i], "-h") == 0) {
			printUsage(argv[0]);
			return 0;
		}
		std::cerr << "Unknown argument: " << argv[i] << '\n';
		printUsage(argv[0]);
		return 1;
	}

	botf::app::ClientApp app;
	return app.run(kPort);
}
