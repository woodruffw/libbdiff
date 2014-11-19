#include "../libbdiff.h"

int main(int argc, char const *argv[])
{
	return lbd_patch("hw1", "test.patch", "hw2.new");
}