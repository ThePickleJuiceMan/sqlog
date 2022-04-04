#include "sqlog.h"


int main()
{
	sqlog * logger;
	create_log("/home/jeffrey/testlog.db", &logger, 1);

	write_log(logger, "This is a test");

	write_log(logger, "Clearing log");
	clear_log(logger, 1);
}
