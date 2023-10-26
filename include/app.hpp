#ifndef APP_H
#define APP_H

#include "window.hpp"

namespace engine {
	class App {
		public:
			static constexpr int WIDTH = 1200;
			static constexpr int HEIGHT = 700;

			void run();

		private:
			Window window_ { WIDTH, HEIGHT, "App" };
	};
}

#endif // APP_H
