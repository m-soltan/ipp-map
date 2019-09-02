#include "map.h"
#include "parser.h"

int main(void) {
	int ans;
	ans = runParser();
	if (ans == OUT_OF_MEMORY)
		return 1;
	else if (ans == 0)
		return 0;
	else
		assert(false);
}
