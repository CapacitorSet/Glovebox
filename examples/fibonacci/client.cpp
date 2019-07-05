#include <glovebox.h>
#include <rpc/client.h>
#include <rpc/rpc_error.h>

ClientParams default_client_params;
WeakParams default_weak_params;

int main() {
	ClientKey key = read_client_key("secret.key");
	if (key == nullptr) {
		puts("secret.key not found: run ./keygen first.");
		return 1;
	}
	default_weak_params = default_client_params = ClientParams(key);

	puts("Constructing values...");
	// Automatic encryption. Fancy, huh?
	Int8 a = 1;
	Int8 b = 1;

	puts("Connecting to server...");
	rpc::client client("127.0.0.1", 8000);
	puts("Awaiting result...");
	// Watch this: automatic deserialization.
	Int8 output = client.call("fibonacci", 3, a.serialize(), b.serialize())
	                  .as<std::string>();
	printf("Result: %i\n", output.toInt());

	free_client_key(key);
	return 0;
}
