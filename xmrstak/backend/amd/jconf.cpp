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
#include "xmrstak/misc/jext.hpp"
#include "xmrstak/misc/console.hpp"

#include <cpuid.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


namespace xmrstak
{
namespace amd
{

using namespace rapidjson;

/*
 * This enum needs to match index in oConfigValues, otherwise we will get a runtime error
 */
enum configEnum { aGpuThreadsConf, iPlatformIdx };

struct configVal {
	configEnum iName;
	const char* sName;
	Type iType;
};

//Same order as in configEnum, as per comment above
configVal oConfigValues[] = {
	{ aGpuThreadsConf, "gpu_threads_conf", kArrayType },
	{ iPlatformIdx, "platform_index", kNumberType }
};

constexpr size_t iConfigCnt = (sizeof(oConfigValues)/sizeof(oConfigValues[0]));

inline bool checkType(Type have, Type want)
{
	if(want == have)
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
	if(id >= prv->configValues[aGpuThreadsConf]->Size())
		return false;

	const Value& oThdConf = prv->configValues[aGpuThreadsConf]->GetArray()[id];

	if(!oThdConf.IsObject())
		return false;

	const Value *idx, *intensity, *w_size, *aff, *stridedIndex;
	idx = GetObjectMember(oThdConf, "index");
	intensity = GetObjectMember(oThdConf, "intensity");
	w_size = GetObjectMember(oThdConf, "worksize");
	aff = GetObjectMember(oThdConf, "affine_to_cpu");
	stridedIndex = GetObjectMember(oThdConf, "strided_index");

	if(idx == nullptr || intensity == nullptr || w_size == nullptr || aff == nullptr || stridedIndex == nullptr)
		return false;

	if(!idx->IsUint64() || !intensity->IsUint64() || !w_size->IsUint64())
		return false;

	if(!aff->IsUint64() && !aff->IsBool())
		return false;

	if(!stridedIndex->IsBool())
		return false;

	cfg.index = idx->GetUint64();
	cfg.intensity = intensity->GetUint64();
	cfg.w_size = w_size->GetUint64();
	cfg.stridedIndex = stridedIndex->GetBool();

	if(aff->IsNumber())
		cfg.cpu_aff = aff->GetInt64();
	else
		cfg.cpu_aff = -1;

	return true;
}

size_t jconf::GetPlatformIdx()
{
	return prv->configValues[iPlatformIdx]->GetUint64();
}

size_t jconf::GetThreadCount()
{
	return prv->configValues[aGpuThreadsConf]->Size();
}

bool jconf::parse_config(const char* sFilename)
{
	FILE * pFile;
	char * buffer;
	size_t flen;

	pFile = fopen(sFilename, "rb");
	if (pFile == NULL)
	{
		printer::inst()->print_msg(L0, "Failed to open config file %s.", sFilename);
		return false;
	}

	fseek(pFile,0,SEEK_END);
	flen = ftell(pFile);
	rewind(pFile);

	if(flen >= 64*1024)
	{
		fclose(pFile);
		printer::inst()->print_msg(L0, "Oversized config file - %s.", sFilename);
		return false;
	}

	if(flen <= 16)
	{
		printer::inst()->print_msg(L0, "File is empty or too short - %s.", sFilename);
		return false;
	}

	buffer = (char*)malloc(flen + 3);
	if(fread(buffer+1, flen, 1, pFile) != 1)
	{
		free(buffer);
		fclose(pFile);
		printer::inst()->print_msg(L0, "Read error while reading %s.", sFilename);
		return false;
	}
	fclose(pFile);

	//Replace Unicode BOM with spaces - we always use UTF-8
	unsigned char* ubuffer = (unsigned char*)buffer;
	if(ubuffer[1] == 0xEF && ubuffer[2] == 0xBB && ubuffer[3] == 0xBF)
	{
		buffer[1] = ' ';
		buffer[2] = ' ';
		buffer[3] = ' ';
	}

	buffer[0] = '{';
	buffer[flen] = '}';
	buffer[flen + 1] = '\0';

	prv->jsonDoc.Parse<kParseCommentsFlag|kParseTrailingCommasFlag>(buffer, flen+2);
	free(buffer);

	if(prv->jsonDoc.HasParseError())
	{
		printer::inst()->print_msg(L0, "JSON config parse error(offset %llu): %s",
			int_port(prv->jsonDoc.GetErrorOffset()), GetParseError_En(prv->jsonDoc.GetParseError()));
		return false;
	}


	if(!prv->jsonDoc.IsObject())
	{ //This should never happen as we created the root ourselves
		printer::inst()->print_msg(L0, "Invalid config file. No root?\n");
		return false;
	}

	for(size_t i = 0; i < iConfigCnt; i++)
	{
		if(oConfigValues[i].iName != i)
		{
			printer::inst()->print_msg(L0, "Code error. oConfigValues are not in order.");
			return false;
		}

		prv->configValues[i] = GetObjectMember(prv->jsonDoc, oConfigValues[i].sName);

		if(prv->configValues[i] == nullptr)
		{
			printer::inst()->print_msg(L0, "Invalid config file. Missing value \"%s\".", oConfigValues[i].sName);
			return false;
		}

		if(!checkType(prv->configValues[i]->GetType(), oConfigValues[i].iType))
		{
			printer::inst()->print_msg(L0, "Invalid config file. Value \"%s\" has unexpected type.", oConfigValues[i].sName);
			return false;
		}
	}

	size_t n_thd = prv->configValues[aGpuThreadsConf]->Size();
	thd_cfg c;
	for(size_t i=0; i < n_thd; i++)
	{
		if(!GetThreadConfig(i, c))
		{
			printer::inst()->print_msg(L0, "Thread %llu has invalid config.", int_port(i));
			return false;
		}
	}
	return true;
}

} // namespace amd
} // namespace xmrstak
