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

#include "xmrstak/misc/executor.hpp"
#include "xmrstak/backend/miner_work.hpp"
#include "xmrstak/backend/globalStates.hpp"
#include "xmrstak/backend/backendConnector.hpp"
#include "xmrstak/jconf.hpp"
#include "xmrstak/misc/console.hpp"
#include "xmrstak/params.hpp"
#include "xmrstak/misc/configEditor.hpp"
#include "xmrstak/version.hpp"
#include "xmrstak/misc/utility.hpp"

#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <iostream>
#include <time.h>
#include <iostream>

#include <openssl/ssl.h>
#include <openssl/err.h>

#include <boost/program_options.hpp>

namespace {
	const size_t ERROR_IN_COMMAND_LINE = 1;
	const size_t SUCCESS = 0;
	const size_t ERROR_UNHANDLED_EXCEPTION = 2;
} // namespace

struct CliConfig {
	bool help;
	bool version;
	bool version_long;
	bool benchmark;
	std::string config;
	std::string currency;
	bool no_cpu;
	std::string cpu_config;
	bool no_amd;
	std::string amd_config;
	bool no_nvidia;
	std::string nvidia_config;
	std::string url;
	std::string user;
	std::string pass;
	bool use_nicehash;
};

int main(int argc, char **argv) {
	try {
		CliConfig cli_config;

		/** Define and parse the program options */
		namespace po = boost::program_options;
		po::options_description desc("Options");
		desc.add_options()
				("help", po::value<bool>(&cli_config.help)->implicit_value(false), "Print help messages")
				("version", po::value<bool>(&cli_config.version)->implicit_value(false), "show version number")
				("version-long", po::value<bool>(&cli_config.version_long)->implicit_value(false), "show long version number")
				("benchmark", po::value<bool>(&cli_config.benchmark)->implicit_value(false), "run the benchmark + tests")
				("config", po::value<std::string>(&cli_config.config)->implicit_value("config.txt"), "common miner configuration file")
				("currency", po::value<std::string>(&cli_config.currency)->implicit_value("monero"), "currency to mine: monero or aeon")
//
				("no-cpu", po::value<bool>(&cli_config.no_cpu)->implicit_value(false), "disable the CPU miner backend")
				("cpu-config", po::value<std::string>(&cli_config.cpu_config)->implicit_value("cpu.txt"), "CPU backend miner config file")
//
				("no-amd", po::value<bool>(&cli_config.no_amd)->implicit_value(false), "disable the AMD miner backend")
				("amd-config", po::value<std::string>(&cli_config.amd_config)->implicit_value("amd.txt"), "AMD backend miner config file")
//
				("no-nvidia", po::value<bool>(&cli_config.no_nvidia)->implicit_value(false), "disable the NVIDIA miner backend")
				("nvidia-config", po::value<std::string>(&cli_config.nvidia_config)->implicit_value("nvidia.txt"), "NVIDIA backend miner config file")
//
				("url", po::value<std::string>(&cli_config.url)->implicit_value(""), "pool url and port, e.g. pool.usxmrpool.com:3333")
				("user", po::value<std::string>(&cli_config.user)->implicit_value(""), "pool user name or wallet address")
				("pass", po::value<std::string>(&cli_config.pass)->implicit_value(""), "pool password, in the most cases x or empty \"\"")
//
				("use-nicehash", po::value<bool>(&cli_config.use_nicehash)->implicit_value(false), "the pool should run in nicehash mode");

		po::variables_map vm;
		try {
			po::store(po::parse_command_line(argc, argv, desc), vm); // can throw
			po::notify(vm); // throws on error, so do after help in case
		}
		catch (po::error &e) {
			std::cerr << "ERROR: " << e.what() << std::endl << std::endl;
			std::cerr << desc << std::endl;
			return ERROR_IN_COMMAND_LINE;
		}

		if (cli_config.help) {
			std::cout << "Basic Command Line Parameter App" << std::endl << desc << std::endl;
			return SUCCESS;
		}
	}
	catch (std::exception &e) {
		std::cerr << "Unhandled Exception reached the top of main: " << e.what() << ", application will now exit" << std::endl;
		return ERROR_UNHANDLED_EXCEPTION;
	}
	return SUCCESS;
} // main



/*
int main(int argc, char *argv[]) {
	SSL_library_init();
	SSL_load_error_strings();
	ERR_load_BIO_strings();
	ERR_load_crypto_strings();
	SSL_load_error_strings();
	OpenSSL_add_all_digests();

	srand(time(0));

	using namespace xmrstak;

	std::string pathWithName(argv[0]);
	std::string seperator("/");
	auto pos = pathWithName.rfind(seperator);

	if (pos == std::string::npos) {
		// try windows "\"
		seperator = "\\";
		pos = pathWithName.rfind(seperator);
	}
	params::inst().binaryName = std::string(pathWithName, pos + 1, std::string::npos);
	if (params::inst().binaryName.compare(pathWithName) != 0) {
		params::inst().executablePrefix = std::string(pathWithName, 0, pos);
		params::inst().executablePrefix += seperator;
	}

	bool uacDialog = true;
	bool pool_url_set = false;
	for (size_t i = 1; i < argc - 1; i++) {
		std::string opName(argv[i]);
		if (opName == "-o" || opName == "-O" || opName == "--url" || opName == "--tls-url")
			pool_url_set = true;
	}

	for (size_t i = 1; i < argc; ++i) {
		std::string opName(argv[i]);
		if (opName.compare("-h") == 0 || opName.compare("--help") == 0) {
			help();
			win_exit(0);
			return 0;
		}
		if (opName.compare("-v") == 0 || opName.compare("--version") == 0) {
			std::cout << "Version: " << get_version_str_short() << std::endl;
			win_exit();
			return 0;
		}
		if (opName.compare("-V") == 0 || opName.compare("--version-long") == 0) {
			std::cout << "Version: " << get_version_str() << std::endl;
			win_exit();
			return 0;
		}
		if (opName.compare("--benchmark") == 0) {
			do_benchmark();
			win_exit(0);
			return 0;
		}

		if (opName.compare("--noCPU") == 0) {
			params::inst().useCPU = false;
		} else if (opName.compare("--noAMD") == 0) {
			params::inst().useAMD = false;
		} else if (opName.compare("--noNVIDIA") == 0) {
			params::inst().useNVIDIA = false;
		} else if (opName.compare("--cpu") == 0) {
			++i;
			if (i >= argc) {
				printer::inst()->print_msg(L0, "No argument for parameter '--cpu' given");
				win_exit();
				return 1;
			}
			params::inst().configFileCPU = argv[i];
		} else if (opName.compare("--amd") == 0) {
			++i;
			if (i >= argc) {
				printer::inst()->print_msg(L0, "No argument for parameter '--amd' given");
				win_exit();
				return 1;
			}
			params::inst().configFileAMD = argv[i];
		} else if (opName.compare("--nvidia") == 0) {
			++i;
			if (i >= argc) {
				printer::inst()->print_msg(L0, "No argument for parameter '--nvidia' given");
				win_exit();
				return 1;
			}
			params::inst().configFileNVIDIA = argv[i];
		} else if (opName.compare("--currency") == 0) {
			++i;
			if (i >= argc) {
				printer::inst()->print_msg(L0, "No argument for parameter '--currency' given");
				win_exit();
				return 1;
			}
			params::inst().currency = argv[i];
		} else if (opName.compare("-o") == 0 || opName.compare("--url") == 0) {
			++i;
			if (i >= argc) {
				printer::inst()->print_msg(L0, "No argument for parameter '-o/--url' given");
				win_exit();
				return 1;
			}
			params::inst().poolURL = argv[i];
			params::inst().poolUseTls = false;
		} else if (opName.compare("-O") == 0 || opName.compare("--tls-url") == 0) {
			++i;
			if (i >= argc) {
				printer::inst()->print_msg(L0, "No argument for parameter '-O/--tls-url' given");
				win_exit();
				return 1;
			}
			params::inst().poolURL = argv[i];
			params::inst().poolUseTls = true;
		} else if (opName.compare("-u") == 0 || opName.compare("--user") == 0) {
			if (!pool_url_set) {
				printer::inst()->print_msg(L0,
										   "Pool address has to be set if you want to specify username and password.");
				win_exit();
				return 1;
			}

			++i;
			if (i >= argc) {
				printer::inst()->print_msg(L0, "No argument for parameter '-u/--user' given");
				win_exit();
				return 1;
			}
			params::inst().poolUsername = argv[i];
		} else if (opName.compare("-p") == 0 || opName.compare("--pass") == 0) {
			if (!pool_url_set) {
				printer::inst()->print_msg(L0,
										   "Pool address has to be set if you want to specify username and password.");
				win_exit();
				return 1;
			}

			++i;
			if (i >= argc) {
				printer::inst()->print_msg(L0, "No argument for parameter '-p/--pass' given");
				win_exit();
				return 1;
			}
			params::inst().userSetPwd = true;
			params::inst().poolPasswd = argv[i];
		} else if (opName.compare("--use-nicehash") == 0) {
			params::inst().nicehashMode = true;
		} else if (opName.compare("-c") == 0 || opName.compare("--config") == 0) {
			++i;
			if (i >= argc) {
				printer::inst()->print_msg(L0, "No argument for parameter '-c/--config' given");
				win_exit();
				return 1;
			}
			params::inst().configFile = argv[i];
		} else if (opName.compare("--noUAC") == 0) {
			uacDialog = false;
		} else {
			printer::inst()->print_msg(L0, "Parameter unknown '%s'", argv[i]);
			win_exit();
			return 1;
		}
	}

	if (!jconf::inst()->parse_config(params::inst().configFile.c_str())) {
		win_exit();
		return 1;
	}

	if (!BackendConnector::self_test()) {
		win_exit();
		return 1;
	}

	printer::inst()->print_str("-------------------------------------------------------------------\n");
	printer::inst()->print_str(get_version_str_short().c_str());
	printer::inst()->print_str("\n\n");
	printer::inst()->print_str("Brought to you by fireice_uk and psychocrypt under GPLv3.\n");
	printer::inst()->print_str("Based on CPU mining code by wolf9466 (heavily optimized by fireice_uk).\n");
#ifndef CONF_NO_CUDA
	printer::inst()->print_str("Based on NVIDIA mining code by KlausT and psychocrypt.\n");
#endif
#ifndef CONF_NO_OPENCL
	printer::inst()->print_str("Based on OpenCL mining code by wolf9466.\n");
#endif
	printer::inst()->print_str("You can use following keys to display reports:\n");
	printer::inst()->print_str("'h' - hashrate\n");
	printer::inst()->print_str("'r' - results\n");
	printer::inst()->print_str("'c' - connection\n");
	printer::inst()->print_str("-------------------------------------------------------------------\n");
	if (::jconf::inst()->IsCurrencyMonero())
		printer::inst()->print_msg(L0, "Start mining: MONERO");
	else
		printer::inst()->print_msg(L0, "Start mining: AEON");

	if (strlen(jconf::inst()->GetOutputFile()) != 0)
		printer::inst()->open_logfile(jconf::inst()->GetOutputFile());

	executor::inst()->ex_start(jconf::inst()->DaemonMode());

	using namespace std::chrono;
	uint64_t lastTime = time_point_cast<milliseconds>(high_resolution_clock::now()).time_since_epoch().count();

	int key;
	while (true) {
		key = get_key();

		switch (key) {
			case 'h':
				executor::inst()->push_event(ex_event(EV_USR_HASHRATE));
				break;
			case 'r':
				executor::inst()->push_event(ex_event(EV_USR_RESULTS));
				break;
			case 'c':
				executor::inst()->push_event(ex_event(EV_USR_CONNSTAT));
				break;
			default:
				break;
		}

		uint64_t currentTime = time_point_cast<milliseconds>(high_resolution_clock::now()).time_since_epoch().count();

		// Hard guard to make sure we never get called more than twice per second
		if (currentTime - lastTime < 500)
			std::this_thread::sleep_for(std::chrono::milliseconds(500 - (currentTime - lastTime)));
		lastTime = currentTime;
	}

	return 0;
}
*/

void do_benchmark() {
	using namespace std::chrono;
	std::vector < xmrstak::iBackend * > *pvThreads;

	printer::inst()->print_msg(L0, "Running a 60 second benchmark...");

	uint8_t work[76] = {0};
	xmrstak::miner_work oWork = xmrstak::miner_work("", work, sizeof(work), 0, false, 0);
	pvThreads = xmrstak::BackendConnector::thread_starter(oWork);

	uint64_t iStartStamp = time_point_cast<milliseconds>(high_resolution_clock::now()).time_since_epoch().count();

	std::this_thread::sleep_for(std::chrono::seconds(60));

	oWork = xmrstak::miner_work();
	xmrstak::pool_data dat;
	xmrstak::globalStates::inst().switch_work(oWork, dat);

	double fTotalHps = 0.0;
	for (uint32_t i = 0; i < pvThreads->size(); i++) {
		double fHps = pvThreads->at(i)->iHashCount;
		fHps /= (pvThreads->at(i)->iTimestamp - iStartStamp) / 1000.0;

		printer::inst()->print_msg(L0, "Thread %u: %.1f H/S", i, fHps);
		fTotalHps += fHps;
	}

	printer::inst()->print_msg(L0, "Total: %.1f H/S", fTotalHps);
}
