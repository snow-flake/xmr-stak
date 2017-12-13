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

#ifndef CONF_NO_HTTPD


#include "httpd.hpp"
#include "webdesign.hpp"
#include "xmrstak/net/msgstruct.hpp"
#include "xmrstak/misc/console.hpp"
#include "xmrstak/misc/executor.hpp"
#include "xmrstak/jconf.hpp"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <string>

#include <microhttpd.h>
#ifdef _WIN32
#define strcasecmp _stricmp
#endif // _WIN32

httpd* httpd::oInst = nullptr;

httpd::httpd()
{

}

int httpd::req_handler(void * cls,
			MHD_Connection* connection,
			const char* url,
			const char* method,
			const char* version,
			const char* upload_data,
			size_t* upload_data_size,
			void ** ptr)
{
	struct MHD_Response * rsp;

	if (strcmp(method, "GET") != 0)
		return MHD_NO;

	if(strlen(jconf::inst()->GetHttpUsername()) != 0)
	{
		char* username;
		int ret;

		username = MHD_digest_auth_get_username(connection);
		if (username == NULL)
		{
			rsp = MHD_create_response_from_buffer(sHtmlAccessDeniedSize, (void*)sHtmlAccessDenied, MHD_RESPMEM_PERSISTENT);
			ret = MHD_queue_auth_fail_response(connection, sHttpAuthRelam, sHttpAuthOpaque, rsp, MHD_NO);
			MHD_destroy_response(rsp);
			return ret;
		}
		free(username);

		ret = MHD_digest_auth_check(connection, sHttpAuthRelam, jconf::inst()->GetHttpUsername(), jconf::inst()->GetHttpPassword(), 300);
		if (ret == MHD_INVALID_NONCE || ret == MHD_NO)
		{
			rsp = MHD_create_response_from_buffer(sHtmlAccessDeniedSize, (void*)sHtmlAccessDenied, MHD_RESPMEM_PERSISTENT);
			ret = MHD_queue_auth_fail_response(connection, sHttpAuthRelam, sHttpAuthOpaque, rsp, (ret == MHD_INVALID_NONCE) ? MHD_YES : MHD_NO);
			MHD_destroy_response(rsp);
			return ret;
		}
	}

	int ret = MHD_queue_response(connection, MHD_HTTP_OK, rsp);
	MHD_destroy_response(rsp);
	return ret;
}

bool httpd::start_daemon()
{
	d = MHD_start_daemon(MHD_USE_THREAD_PER_CONNECTION,
		jconf::inst()->GetHttpdPort(), NULL, NULL,
		&httpd::req_handler,
		NULL, MHD_OPTION_END);

	if(d == nullptr)
	{
		printer::inst()->print_str("HTTP Daemon failed to start.");
		return false;
	}

	return true;
}

#endif

