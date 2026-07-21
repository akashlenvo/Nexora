#pragma once

#include <string>
#include <cstdlib>

#if defined(_WIN32)
#include <windows.h>
#endif

#include "logger.h"

namespace adb
{
	/*
	* Get the local directory of the program
	*/
	std::string dir()
	{
#if defined(_WIN32)
		char buffer[MAX_PATH];
		GetModuleFileNameA(NULL, buffer, MAX_PATH);

		std::string path(buffer);
		return path.substr(0, path.find_last_of("\\/"));
#else
		return {};
#endif
	}

	std::string executable()
	{
#if defined(_WIN32)
		return "\"" + dir() + "\\adb\\adb.exe\"";
#else
		return "adb";
#endif
	}

	bool run(const std::string& arguments)
	{
		return std::system((executable() + " " + arguments).c_str()) == 0;
	}

	
	/*
	* Reverse the given tcp port.
	* adb reverse tcp:<port> tcp:<port>
	*/
	bool reverse(int port)
	{
		if (run("reverse tcp:" + std::to_string(port) + " tcp:" + std::to_string(port)))
		{
			logger << "[ADB:" << port << "] Started " << std::endl;
			return true;
		}
		else
		{
			logger << "[ADB:" << port << "] Failed to start " << std::endl;
			return false;
		}
	}

	/*
	* Forwards a given tcp port
	* adb forward tcp:<port> tcp:<port>
	*/
	bool forward(int port)
	{
		if (run("forward tcp:" + std::to_string(port) + " tcp:" + std::to_string(port)))
		{
			logger << "[ADB:" << port << "] Started " << std::endl;
			return true;
		}
		else
		{
			logger << "[ADB:" << port << "] Failed to start " << std::endl;
			return false;
		}
	}

	/*
	* Removes the reversed given tcp port.
	* adb reverse --remove tcp:<port>
	*/
	bool kill(int port)
	{
		const bool removed = run("reverse --remove tcp:" + std::to_string(port));
		// Stop the ADB server after removing the tunnel so the port is not
		// retained across application restarts.
		run("kill-server");

		if (removed)
		{
			logger << "[ADB:" << port << "] Stopped " << std::endl;
			return true;
		}
		else
		{
			logger << "[ADB:" << port << "] Failed to stop " << std::endl;
			return false;
		}
	}
}
