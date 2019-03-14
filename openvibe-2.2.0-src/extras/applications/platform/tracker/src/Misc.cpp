
#include "Misc.h"

ovtime_t ceilFixedPoint(ovtime_t value)
{
	const bool hasDecimals = (value << 32) > 0;
	return (hasDecimals ? ((value & (0xFFFFFFFFLL << 32)) + (1LL << 32)) : value);
}

