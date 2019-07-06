#include <glovebox.h>
#include <rpc/server.h>

#include "patient.h"

thread_local ServerParams server_params;

int main() {
	ServerKey key = read_server_key("cloud.key");
	if (key == nullptr) {
		puts("cloud.key not found: run ./keygen first.");
		return 1;
	}
	server_params = ServerParams(key);

	Array<Patient, 10> database(false);

	rpc::server srv(8000);

	srv.bind("uploadDatabase", [&](std::string _db) {
		puts("Receiving database...");
		database = _db;
		puts("Database received.");
	});

	srv.bind("countM", [&]() {
		puts("Counting men...");
		return database.countIf([](Patient p) { return p.isMale; }).serialize();
	});

	srv.bind("sumWeight", [&]() {
		puts("Calculating sum of weights...");
		auto weights = database.map<Q7_1>([](Patient p) { return p.weight; });
		auto sumW = sum(weights);
		return sumW.serialize();
	});

	using Q7_9 = Fixed<7, 9>;
	// See regression.m for the calculation; it's basic polynomial interpolation
	const auto heightPoly = std::vector<double>({21.0933654, 0.4819932, 0.0044268});
	const Polynomial<Q7_9> weightModel = Polynomial<Q7_9>(heightPoly);

	srv.bind("predictWeight", [&](std::string _weight) {
		puts("Predicting weight...");
		Q7_9 weight = _weight;
		bit_t overflow = make_bit();
		return weightModel.evaluate(overflow, weight).serialize();
	});

	srv.run();

	return 0;
}
