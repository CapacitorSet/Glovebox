#include <cassert>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <string>
#include <fhe-tools.h>

#include "patient.h"
#include "../networking.h"
#include "../dyad.h"

TFHEClientParams_t default_client_params;

dyad_Stream *s;

constexpr int NUM_PATIENTS = 10;
Array<Patient, NUM_PATIENTS>* records;

void onConnect(dyad_Event*) {
	puts("Sending...");
	send(s, records);
}

void onPacket(dyad_Stream*, char *packet, size_t pktsize, char dataType) {
}

int main() {
	FILE *secret_key = fopen("secret.key", "rb");
	if (secret_key == nullptr) {
		puts("secret.key not found: run ./keygen first.");
		return 1;
	}
	default_client_params = makeTFHEClientParams(secret_key);
	fclose(secret_key);

	puts("Reading data...");
	std::ifstream file("examples/health/data.csv");
	std::string line;

	std::getline(file, line);
	assert(line == "height,weight,age,male");

	records = new Array<Patient, NUM_PATIENTS>(true, default_client_params);

	for (int i = 0; i < NUM_PATIENTS; i++) {
		std::getline(file, line);
		double height;
		double weight;
		int8_t age;
		char isMale;
		sscanf(line.c_str(), "%lf,%lf,%d,%c\n", &height, &weight, &age, &isMale);
		printf("%lf,%lf,%d,%d\n", height, weight, age, isMale == '1');
		Patient p(height, weight, age, isMale == '1', default_client_params);
		records->put(p, i);
	}

	dyad_init();

	s = dyad_newStream();
	dyad_addListener(s, DYAD_EVENT_CONNECT, onConnect, nullptr);
	dyad_addListener(s, DYAD_EVENT_DATA, onData, nullptr);
	puts("Connecting...");
	dyad_connect(s, "127.0.0.1", 8000);

	while (dyad_getStreamCount() > 0) {
		dyad_update();
	}
	puts("No more connections, closing.");

	dyad_shutdown();
	freeTFHEClientParams(default_client_params);
	delete records;
	return 0;
}
