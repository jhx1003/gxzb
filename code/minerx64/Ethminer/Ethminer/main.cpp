/*
	This file is part of cpp-ethereum.

	cpp-ethereum is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	cpp-ethereum is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with cpp-ethereum.  If not, see <http://www.gnu.org/licenses/>.
*/
/** @file main.cpp
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 * Ethereum client.
 */



// Solves the problem of including windows.h before including winsock.h
// as detailed here:
// http://stackoverflow.com/questions/1372480/c-redefinition-header-files-winsock2-h

#if defined(_WIN32)
	#define _WINSOCKAPI_
	#include <windows.h>
#endif // defined(_WIN32)

#include <stdio.h>
#include <clocale>
char g_szDagPath[256] = { 0 };

#include <tchar.h>
#include "MinerAux.h"
#include "MinerHelper.h"

using namespace std;
using namespace dev;
using namespace dev::eth;
using namespace boost::algorithm;

#undef RETURN

void help()
{
	cout
		<< "Usage ethminer [OPTIONS]" << endl
		<< "Options:" << endl << endl;
	MinerCLI::streamHelp(cout);
	cout
		<< "General Options:" << endl
		<< "    -v,--verbosity <0 - 9>  Set the log verbosity from 0 to 9 (default: 8)." << endl
		<< "    -V,--version  Show the version and exit." << endl
		<< "    -h,--help  Show this help message and exit." << endl
		;
	exit(0);
}

void version()
{
	cout << "ethminer version " << dev::Version << endl;
	cout << "Build: " << DEV_QUOTED(ETH_BUILD_PLATFORM) << "/" << DEV_QUOTED(ETH_BUILD_TYPE) << endl;
	exit(0);
}

/*
The equivalent of setlocale(LC_ALL, “C”) is called before any user code is run.
If the user has an invalid environment setting then it is possible for the call
to set locale to fail, so there are only two possible actions, the first is to
throw a runtime exception and cause the program to quit (default behaviour),
or the second is to modify the environment to something sensible (least
surprising behaviour).

The follow code produces the least surprising behaviour. It will use the user
specified default locale if it is valid, and if not then it will modify the
environment the process is running in to use a sensible default. This also means
that users do not need to install language packs for their OS.
*/
void setDefaultOrCLocale()
{
#if __unix__
	if (!std::setlocale(LC_ALL, ""))
	{
		setenv("LC_ALL", "C", 1);
	}
#endif
}

int main(int argc, char** argv)
{
	setDefaultOrCLocale();
	if (MsgWindow::HandleSingleton()) {
		exit(0);
		return 0;
	}
	unique_ptr<thread> threadMiner;
	MinerCLI m(MinerCLI::OperationMode::Farm);
	threadMiner.reset(new thread([&]() {
		setThreadName("workertd");
		for (int i = 1; i < argc; ++i)
		{
			string arg = argv[i];
			if (m.interpretOption(i, argc, argv))
			{
			}
			else if ((arg == "-v" || arg == "--verbosity") && i + 1 < argc)
				g_logVerbosity = atoi(argv[++i]);
			else if (arg == "-h" || arg == "--help")
				help();
			else if (arg == "-V" || arg == "--version")
				version();
			else
			{
				cerr << "Invalid argument: " << arg << endl;
				exit(-1);
			}
		}

		m.execute();
	}));
	//消息窗口已经存在 或 只有1个命令行时不用创建消息窗口
	if (argc > 2)
	{
		MsgWindow::Create();
	}
	return 0;
}
