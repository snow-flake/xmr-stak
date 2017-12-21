/*
  * This program is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * any later version.
  *
  * This program is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  * GNU General Public License for more details.
  *
  * You should have received a copy of the GNU General Public License
  * along with this program.  If not, see <http://www.gnu.org/licenses/>.
  *
  * Additional permission under GNU GPL version 3 section 7
  *
  * If you modify this Program, or any covered work, by linking or combining
  * it with OpenSSL (or a modified version of that library), containing parts
  * covered by the terms of OpenSSL License and SSLeay License, the licensors
  * of this Program grant you additional permission to convey the resulting work.
  *
  */

#include "jconf.hpp"
#include "xmrstak/misc/console.hpp"
#include "xmrstak/misc/jext.hpp"

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <cpuid.h>


namespace xmrstak
{
namespace cpu
{

using namespace rapidjson;

/*
 * This enum needs to match index in oConfigValues, otherwise we will get a runtime error
 */
enum configEnum { aCpuThreadsConf, sUseSlowMem };

struct configVal {
	configEnum iName;
	const char* sName;
	Type iType;
};

// Same order as in configEnum, as per comment above
// kNullType means any type
configVal oConfigValues[] = {
	{ aCpuThreadsConf, "cpu_threads_conf", kNullType }
};

constexpr size_t iConfigCnt = (sizeof(oConfigValues)/sizeof(oConfigValues[0]));

inline bool checkType(Type have, Type want)
{
	if(want == have)
		return true;
	else if(want == kNullType)
		return true;
	else if(want == kTrueType && have == kFalseType)
		return true;
	else if(want == kFalseType && have == kTrueType)
		return true;
	else
		return false;
}

struct jconf::opaque_private
{
	Document jsonDoc;
	const Value* configValues[iConfigCnt]; //Compile time constant

	opaque_private()
	{
	}
};

jconf* jconf::oInst = nullptr;

jconf::jconf()
{
	prv = new opaque_private();
}

bool jconf::GetThreadConfig(size_t id, thd_cfg &cfg)
{
	if(!prv->configValues[aCpuThreadsConf]->IsArray())
		return false;

	if(id >= prv->configValues[aCpuThreadsConf]->Size())
		return false;

	const Value& oThdConf = prv->configValues[aCpuThreadsConf]->GetArray()[id];

	if(!oThdConf.IsObject())
		return false;

	const Value *mode, *no_prefetch, *aff;
	mode = GetObjectMember(oThdConf, "low_power_mode");
	no_prefetch = GetObjectMember(oThdConf, "no_prefetch");
	aff = GetObjectMember(oThdConf, "affine_to_cpu");

	if(mode == nullptr || no_prefetch == nullptr || aff == nullptr)
		return false;

	if(!mode->IsBool() && !mode->IsNumber())
		return false;

	if(!no_prefetch->IsBool())
		return false;

	if(!aff->IsNumber() && !aff->IsBool())
		return false;

	if(aff->IsNumber() && aff->GetInt64() < 0)
		return false;

	if(mode->IsNumber())
		cfg.iMultiway = (int)mode->GetInt64();
	else
		cfg.iMultiway = mode->GetBool() ? 2 : 1;

	cfg.bNoPrefetch = no_prefetch->GetBool();

	if(aff->IsNumber())
		cfg.iCpuAff = aff->GetInt64();
	else
		cfg.iCpuAff = -1;

	return true;
}


size_t jconf::GetThreadCount()
{
	if(prv->configValues[aCpuThreadsConf]->IsArray())
		return prv->configValues[aCpuThreadsConf]->Size();
	else
		return 0;
}

} // namespace cpu
} // namepsace xmrstak
