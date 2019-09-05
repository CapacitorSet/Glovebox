#ifndef GLOVEBOX_PARALLEL_H
#define GLOVEBOX_PARALLEL_H

#include <iostream>
#include <rpc/client.h>
#include <thread>

using parallel_host_t = struct {
	std::string address;
	uint16_t port;
};

extern std::vector<parallel_host_t> parallel_hosts;

// Essentially the map stage in map-reduce.
// Takes in a vector<T>, returns a "mask" (true = satisfies the filter).
template <class T, class... Args>
bitvec_t filter(const std::vector<T> src, const std::string &fnName, Args... args) {
	size_t numHosts = parallel_hosts.size();
	assert(numHosts != 0);
	bitvec_t results = make_bitvec(src.size());

	std::mutex m;
	std::condition_variable cv;
	int semaphore = numHosts;
	size_t next_available_item = 0;

	for (const auto &host : parallel_hosts) {
		/* Create one thread for each host. Each thread will connect to the
		 * host, send it tasks as long as there are available ones, and write
		 * the results into the `results` mask.
		 * The main thread waits for them with a semaphore.
		 */
		auto handler = std::thread([&, host] {
			rpc::client client(host.address, host.port);
			while (true) {
				// Fetch a new item from the work queue as soon as we're free
				// If the queue is empty, signal that we're done and exit
				std::unique_lock<std::mutex> lk(m);
				if (next_available_item >= src.size())
					break;
				size_t i = next_available_item++;
				lk.unlock();

				// std::cout << "Calling item " << i << " on host " <<
				// host.address << ":" << host.port << std::endl;
				std::string str_tmp =
				    client.call(fnName, src[i], args...).template as<std::string>();
				bit_t tmp = make_bit(str_tmp);
				_copy(results[i], tmp);
			}
			std::unique_lock<std::mutex> lk(m);
			semaphore--;
			lk.unlock();
			cv.notify_one();
		});
		handler.detach();
	}

	// Wait for every thread to be finished
	std::unique_lock<std::mutex> lk(m);
	cv.wait(lk, [&] { return semaphore == 0; });

	return results;
}

template <class T, class... Args>
bit_t any_of(const std::vector<T> src, const std::string &fnName, Args... args) {
	bitvec_t results = filter(src, fnName, args...);

	// Reduce the results
	bit_t ret = make_bit();
	_unsafe_constant(ret, false);
	for (const auto &result : results)
		_or(ret, ret, result);
	return ret;
}

template <class T, class... Args>
bit_t all_of(const std::vector<T> src, const std::string &fnName, Args... args) {
	bitvec_t results = filter(src, fnName, args...);

	// Reduce the results
	bit_t ret = make_bit();
	_unsafe_constant(ret, true);
	for (const auto &result : results)
		_and(ret, ret, result);
	return ret;
}

#endif // GLOVEBOX_PARALLEL_H
