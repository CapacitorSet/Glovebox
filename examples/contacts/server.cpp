#include <cassert>
#include <cstdio>
#include <cstring>
#include <fhe-tools.h>
#include <rpc/server.h>

#include "contact.h"

TFHEServerParams_t default_server_params;

Contact contacts[] = {
    {"", 504'305'6784}, {"", 208'348'4604}, {"", 713'729'7840},
    {"", 714'543'1510}, {"", 907'553'8994}, {"", 704'551'2763},
    {"", 172'265'8920}, {"", 583'642'3948}, {"", 754'768'2428},
    {"", 921'915'3647}, {"", 790'793'8139}, {"", 936'327'0855},
};

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

	srv.bind("isKnownContact", [](std::string _userNumber) {
		puts("Received request.");
		PhoneNumber userNumber(_userNumber);
		bit_t isKnown = make_bit();
		constant(isKnown, false);
		for (const auto &contact : contacts) {
			PhoneNumber contactNo(contact.phoneNumber);
			bit_t matches = equals(contactNo.data, userNumber.data);
			_or(isKnown, isKnown, matches);
		}
		return exportToString(isKnown, default_server_params);
	});

	srv.run();

	return 0;
}
