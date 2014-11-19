#include "../libbdiff.h"

int main(int argc, char const *argv[])
{
	return lbd_diff("hw1", "hw2", "test.patch");
}