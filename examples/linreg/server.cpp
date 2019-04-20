#include <assert.h>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <fhe-tools.h>
#include <rpc/server.h>

TFHEServerParams_t default_server_params;

using Q4_4 = Fixed<4, 4>;

int main() {
	puts("Initializing TFHE...");
	FILE *cloud_key = fopen("cloud.key", "rb");
	if (cloud_key == nullptr) {
		puts("cloud.key not found: run ./keygen first.");
		return 1;
	}
	default_server_params = makeTFHEServerParams(cloud_key);
	fclose(cloud_key);

	rpc::server srv(8000);

	auto p = Polynomial<Q4_4>(std::vector<double>{1.0, 2.0});

	srv.bind("polyeval", [&](std::string _xs) {
		puts("Received request.");
		Array<Q4_4, 2> xs = _xs;
		Array<Q4_4, 2> ys(false);
		bit_t p_overflow = make_bit();
		for (int i = 0; i < 2; i++) {
			auto x = Q4_4(0);
			xs.get(x, i);
			auto y = p.evaluate(p_overflow, x);
			ys.put(y, i);
		}
		return ys.serialize();
	});

	srv.run();

	return 0;
}
