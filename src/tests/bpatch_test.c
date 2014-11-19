/**
 * Attempts to patch a binary named "hw1" with "test.patch",
 * producing a new binary named "hw2.new".
 */

#include "../libbdiff.h"

int main(int argc, char const *argv[])
{
	return lbd_patch("hw1", "test.patch", "hw2.new");
}