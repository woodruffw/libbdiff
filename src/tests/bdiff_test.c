/**
 * Attempts to generate a binary diff named "test.patch"
 * from two binaries "hw1" and "hw2", returning the function's status.
 */

#include "../libbdiff.h"

int main(int argc, char const *argv[])
{
	return lbd_diff("hw1", "hw2", "test.patch");
}