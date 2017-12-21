#pragma once

#include "xmrstak/params.hpp"

#include <stdlib.h>
#include <string>

namespace xmrstak
{
namespace cpu
{

class jconf
{
public:
	inline static jconf* inst() {
		return oInst;
	};

	inline static jconf* init_inst(int cpu_count, int low_power_mode, bool no_prefetch) {
		return oInst;
	};

	bool parse_config(const char* sFilename = params::inst().configFileCPU.c_str());

	struct thd_cfg {
		int iMultiway;
		bool bNoPrefetch;
		long long iCpuAff;
	};

	inline size_t GetThreadCount() { return cpu_count; }

	inline bool GetThreadConfig(size_t id, thd_cfg &cfg) {
		cfg.iMultiway = low_power_mode;
		cfg.bNoPrefetch = no_prefetch;
		cfg.iCpuAff = id;
	}

private:
	jconf(int cpu_count, int low_power_mode, bool no_prefetch):
			cpu_count(cpu_count),
			low_power_mode(low_power_mode),
			no_prefetch(no_prefetch)
	{}

	static jconf* oInst;

	const int cpu_count;
	const int low_power_mode;
	const bool no_prefetch;
};

} // namespace cpu
} // namepsace xmrstak
