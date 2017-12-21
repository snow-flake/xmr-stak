#pragma once

#include <inttypes.h>
#include <string>

#ifndef POOL_ADDRESS
#define POOL_ADDRESS "xmr-us-east1.nanopool.org"
#endif

#ifndef POOL_PORT
#define POOL_PORT "14444"
#endif

#ifndef POOL_TLS
#define POOL_TLS false
#endif


inline std::string get_version_str()
{
	return std::string("xmr-stak/2.1.0/31a078416042d5492540f4bdb1466a84b690430e/master/lin/cpu/monero/0");
}

inline std::string get_version_str_short()
{
	return std::string("xmr-stak 2.1.0 31a078416042d5492540f4bdb1466a84b690430e");
}

inline std::string get_pool_address() {
	return std::string(POOL_ADDRESS ":" POOL_PORT);
}

inline bool get_bool_is_tls() {
	return POOL_TLS;
}
