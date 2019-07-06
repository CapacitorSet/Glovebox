#include <glovebox.h>
#include <iostream>
#include <rpc/client.h>
#include <rpc/rpc_error.h>

#include "contact.h"

ClientParams client_params;

// Generated with fakenamegenerator.com
Contact contacts[] = {
    {"Helen C. Douglas", 504'305'6784}, {"Rosemary J. Mejia", 208'348'4604},
    {"Katie J. Herman", 570'839'5869},  {"Thomas P. McIntosh", 713'729'7840},
    {"Herman M. North", 704'696'9935},  {"Robert R. Snodgrass", 786'374'9340},
    {"Gregory K. Ogle", 308'362'7046},  {"Clarence R. Goodman", 714'543'1510},
    {"Joann F. Harris", 407'620'9392},  {"Evelyn C. Martinez", 704'551'2763},
};

std::vector<parallel_host_t> parallel_hosts = {
    {"127.0.0.1", 8001},
    {"127.0.0.1", 8002},
    {"127.0.0.1", 8003},
};

int main() {
	ClientKey key = read_client_key("secret.key");
	if (key == nullptr) {
		puts("secret.key not found: run ./keygen first.");
		return 1;
	}
	client_params = ClientParams(key);

	puts("Connecting to server...");
	rpc::client client("127.0.0.1", 8000);
	puts("Uploading contact list...");
	std::vector<std::string> phoneNumbers;
	for (const auto &contact : contacts) {
		PhoneNumber phoneNumber(contact.phoneNumber, client_params);
		phoneNumbers.push_back(phoneNumber.serialize());
	}
	bitspan_t mask =
	    filter(phoneNumbers, "isKnownContact", client_params);
	for (int i = 0; i < 10; i++) {
		std::cout << (decrypt(mask[i]) ? "Known: " : "Not known: ")
		          << contacts[i].name << std::endl;
	}
	return 0;
}
