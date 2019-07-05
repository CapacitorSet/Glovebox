#include <cassert>
#include <fstream>
#include <glovebox.h>
#include <rpc/client.h>

#include "patient.h"

ClientParams default_client_params;
WeakParams default_weak_params;

constexpr int NUM_PATIENTS = 10;

int main() {
	ClientKey key = read_client_key("secret.key");
	if (key == nullptr) {
		puts("secret.key not found: run ./keygen first.");
		return 1;
	}
	default_client_params = ClientParams(key);

	puts("Reading data...");
	std::ifstream file("examples/health/data.csv");
	std::string line;

	std::getline(file, line);
	assert(line == "height,weight,age,male");

	auto records = Array<Patient, NUM_PATIENTS>(true, default_client_params);

	for (int i = 0; i < NUM_PATIENTS; i++) {
		std::getline(file, line);
		double height;
		double weight;
		int8_t age;
		char isMale;
		sscanf(line.c_str(), "%lf,%lf,%d,%c\n", &height, &weight, &age,
		       &isMale);
		Patient p(height, weight, age, isMale == '1', default_client_params);
		records.put(p, i);
	}

	puts("Connecting to server...");
	rpc::client client("127.0.0.1", 8000);
	puts("1. Uploading database...");
	client.call("uploadDatabase", records.serialize());
	puts("");

	puts("2. Counting men...");
	Int8 countM = client.call("countM").as<std::string>();
	printf("Output: %d\n", countM.toInt());
	puts("");

	puts("3. Computing average weight...");
	Fixed<11, 1> sum = client.call("sumWeight").as<std::string>();
	// The division is made client-side for performance reasons.
	printf("Output: %lf\n", sum.toDouble() / 10.0);
	puts("");

	// Pick one patient to use for the weight demo
	Patient sample_patient;
	int i = 0;
	do {
		records.get(sample_patient, i++);
	} while (sample_patient.age.toInt() > 20);
	printf("4. Predicting weight for height=%lf...\n",
	       sample_patient.getHeight());
	using Q7_9 = Fixed<7, 9>;
	Q7_9 height = Patient::scaleHeight(sample_patient.getHeight());
	Q7_9 weight =
	    client.call("predictWeight", height.serialize()).as<std::string>();
	printf("Predicted: %lf, actual: %lf\n", weight.toDouble(),
	       sample_patient.weight.toDouble());

	return 0;
}
