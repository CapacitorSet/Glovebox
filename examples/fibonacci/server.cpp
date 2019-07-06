#include <glovebox.h>
#include <rpc/server.h>

thread_local ServerParams server_params;

int main() {
	ServerKey key = read_server_key("cloud.key");
	if (key == nullptr) {
		puts("cloud.key not found: run ./keygen first.");
		return 1;
	}
	server_params = ServerParams(key);

	rpc::server srv(8000);

	srv.bind("fibonacci", [](int times, std::string a, std::string b) {
		puts("Received request.");
		// Note the elegance in automatic deserialization.
		Int8 first = a, second = b;
		Int8 ret = 0;
		for (int i = 0; i < times; i++) {
			printf("Iteration %d\n", i);
			ret.add(first, second);
			// todo: implement copy/move ctor for Int so that the following works
			/*
			first = second;
			second = x;
			*/
			first.copy(second);
			second.copy(ret);
		}
		return ret.serialize();
	});

	srv.run();

	free_server_key(key);
	return 0;
}
