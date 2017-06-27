
#include <cstdio>
#include <mutex>
#include <ctime>
#include <cstdarg>
#include <cstring>
#include "Logging.h"

std::mutex log_locker;

auto flog = fopen("fatwlog.txt", "a");

void glog(const char * format, ...)
{
	if (flog == nullptr)
	{
		fprintf(stderr, "%s\n", "Logging functionality is unavailable.");
		return;
	}

	std::lock_guard<std::mutex> lockguard(log_locker);

	char cbuf[BUFSIZ];
	auto t = std::time(nullptr);
	strftime(cbuf, BUFSIZ, "[%Y/%m/%d %H:%M:%S] ", std::localtime(&t));
	strcat(cbuf, format);

	va_list args;
	va_start(args, format);
	vfprintf(flog, cbuf, args);
	va_end(args);

	return;
}
