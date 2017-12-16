#pragma once

#include "xmrstak/misc/environment.hpp"
#include "params.hpp"

#include <stdlib.h>
#include <string>


class jconf
{
public:
	static jconf* inst()
	{
		auto& env = xmrstak::environment::inst();
		if(env.pJconfConfig == nullptr)
			env.pJconfConfig = new jconf;
		return env.pJconfConfig;
	};

	bool parse_config(const char* sFilename = xmrstak::params::inst().configFile.c_str());

	struct pool_cfg {
		const char* sPoolAddr;
		const char* sWalletAddr;
		const char* sPasswd;
		bool tls;
		const char* tls_fingerprint;
		size_t raw_weight;
		double weight;
	};

	size_t wt_max;
	size_t wt_min;

	uint64_t GetPoolCount();
	bool GetPoolConfig(size_t id, pool_cfg& cfg);

	enum slow_mem_cfg {
		always_use,
		no_mlck,
		print_warning,
		never_use,
		unknown_value
	};

	/*
	 * TLS Settings
	 * If you need real security, make sure tls_secure_algo is enabled (otherwise MITM attack can downgrade encryption
	 * to trivially breakable stuff like DES and MD5), and verify the server's fingerprint through a trusted channel.
	 *
	 * tls_secure_algo - Use only secure algorithms. This will make us quit with an error if we can't negotiate a secure algo.
	 */
	inline bool TlsSecureAlgos() { return true; }

	/*
	 * currency to mine
	 * allowed values: 'monero' or 'aeon'
	 */
	inline const std::string GetCurrency() { return "monero"; }
	inline bool IsCurrencyMonero() { return true; }

	/*
	 * Output control.
	 * Since most people are used to miners printing all the time, that's what we do by default too. This is suboptimal
	 * really, since you cannot see errors under pages and pages of text and performance stats. Given that we have internal
	 * performance monitors, there is very little reason to spew out pages of text instead of concise reports.
	 * Press 'h' (hashrate), 'r' (results) or 'c' (connection) to print reports.
	 *
	 * verbose_level - 0 - Don't print anything.
	 *                 1 - Print intro, connection event, disconnect event
	 *                 2 - All of level 1, and new job (block) event if the difficulty is different from the last job
	 *                 3 - All of level 1, and new job (block) event in all cases, result submission event.
	 *                 4 - All of level 3, and automatic hashrate report printing
	 *
	 * print_motd    - Display messages from your pool operator in the hashrate result.
	 */
	inline uint64_t GetVerboseLevel() { return 3; }
	inline bool PrintMotd() { return true; }

	/*
	 * Automatic hashrate report
	 *
	 * h_print_time - How often, in seconds, should we print a hashrate report if verbose_level is set to 4.
	 *                This option has no effect if verbose_level is not 4.
	 */
	inline uint64_t GetAutohashTime() { return 60; }

	/*
	 * Output file
	 *
	 * output_file  - This option will log all output to a file.
	 *
	 */
	inline const std::string GetOutputFile() { return "/var/log/minerd.log"; }

	/*
	 * Network timeouts.
	 * Because of the way this client is written it doesn't need to constantly talk (keep-alive) to the server to make
	 * sure it is there. We detect a buggy / overloaded server by the call timeout. The default values will be ok for
	 * nearly all cases. If they aren't the pool has most likely overload issues. Low call timeout values are preferable -
	 * long timeouts mean that we waste hashes on potentially stale jobs. Connection report will tell you how long the
	 * server usually takes to process our calls.
	 *
	 * call_timeout - How long should we wait for a response from the server before we assume it is dead and drop the connection.
	 * retry_time	- How long should we wait before another connection attempt.
	 *                Both values are in seconds.
	 * giveup_limit - Limit how many times we try to reconnect to the pool. Zero means no limit. Note that stak miners
	 *                don't mine while the connection is lost, so your computer's power usage goes down to idle.
	 */
	inline uint64_t GetCallTimeout() { return 10; }
	inline uint64_t GetNetRetry() { return 30; }
	inline uint64_t GetGiveUpLimit() { return 0; }

	/*
	 * Daemon mode
	 *
	 * If you are running the process in the background and you don't need the keyboard reports, set this to true.
	 * This should solve the hashrate problems on some emulated terminals.
	 */
	inline bool DaemonMode() { return false; }

	/*
	 * prefer_ipv4 - IPv6 preference. If the host is available on both IPv4 and IPv6 net, which one should be choose?
	 *               This setting will only be needed in 2020's. No need to worry about it now.
	 */
	inline bool PreferIpv4() { return true; }

	inline bool HaveHardwareAes() { return bHaveAes; }

	static void cpuid(uint32_t eax, int32_t ecx, int32_t val[4]);

	/*
	 * LARGE PAGE SUPPORT
	 * Large pages need a properly set up OS. It can be difficult if you are not used to systems administration,
	 * but the performance results are worth the trouble - you will get around 20% boost. Slow memory mode is
	 * meant as a backup, you won't get stellar results there. If you are running into trouble, especially
	 * on Windows, please read the common issues in the README.
	 *
	 * By default we will try to allocate large pages. This means you need to "Run As Administrator" on Windows.
	 * You need to edit your system's group policies to enable locking large pages. Here are the steps from MSDN
	 *
	 * 1. On the Start menu, click Run. In the Open box, type gpedit.msc.
	 * 2. On the Local Group Policy Editor console, expand Computer Configuration, and then expand Windows Settings.
	 * 3. Expand Security Settings, and then expand Local Policies.
	 * 4. Select the User Rights Assignment folder.
	 * 5. The policies will be displayed in the details pane.
	 * 6. In the pane, double-click Lock pages in memory.
	 * 7. In the Local Security Setting – Lock pages in memory dialog box, click Add User or Group.
	 * 8. In the Select Users, Service Accounts, or Groups dialog box, add an account that you will run the miner on
	 * 9. Reboot for change to take effect.
	 *
	 * Windows also tends to fragment memory a lot. If you are running on a system with 4-8GB of RAM you might need
	 * to switch off all the auto-start applications and reboot to have a large enough chunk of contiguous memory.
	 *
	 * On Linux you will need to configure large page support "sudo sysctl -w vm.nr_hugepages=128" and increase your
	 * ulimit -l. To do do this you need to add following lines to /etc/security/limits.conf - "* soft memlock 262144"
	 * and "* hard memlock 262144". You can also do it Windows-style and simply run-as-root, but this is NOT
	 * recommended for security reasons.
	 *
	 * Memory locking means that the kernel can't swap out the page to disk - something that is unlikely to happen on a
	 * command line system that isn't starved of memory. I haven't observed any difference on a CLI Linux system between
	 * locked and unlocked memory. If that is your setup see option "no_mlck".
	 *
	 *
	 * use_slow_memory defines our behaviour with regards to large pages. There are three possible options here:
	 * always  - Don't even try to use large pages. Always use slow memory.
	 * warn    - We will try to use large pages, but fall back to slow memory if that fails.
	 * no_mlck - This option is only relevant on Linux, where we can use large pages without locking memory.
	 *           It will never use slow memory, but it won't attempt to mlock
	 * never   - If we fail to allocate large pages we will print an error and exit.
	 */
	inline slow_mem_cfg GetSlowMemSetting() { return print_warning; }

private:
	jconf();

	bool check_cpu_features();
	struct opaque_private;
	opaque_private* prv;

	bool bHaveAes;
};
