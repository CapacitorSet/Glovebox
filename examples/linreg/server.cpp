#include <cstdlib>
#include <glovebox.h>
#include <rpc/server.h>

thread_local ServerParams server_params;
thread_local WeakParams weak_params;

using Q4_4 = Fixed<4, 4>;

int main() {
	ServerKey key = ServerKey("cloud.key");
	if (key == nullptr) {
		puts("cloud.key not found: run ./keygen first.");
		return 1;
	}
	weak_params = server_params = ServerParams(key);

	rpc::server srv(8000);

	auto p = Polynomial<Q4_4>(std::vector<double>{1.0, 2.0});

	srv.bind("polyeval", [&](std::string _xs) {
		puts("Received request.");
		Array<Q4_4, 2> xs = _xs;
		Array<Q4_4, 2> ys(false);
		bit_t p_overflow = make_bit();
		for (int i = 0; i < 2; i++) {
			Q4_4 x = 0;
			xs.get(x, i);
			auto y = p.evaluate(p_overflow, x);
			ys.put(y, i);
		}
		return ys.serialize();
	});

	srv.run();

	return 0;
}
