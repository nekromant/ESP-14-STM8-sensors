#include <string.h>
#include "string_utils.h"

namespace io {
uint8_t* InsertDot(uint16_t value, uint8_t position, uint8_t* buf)
		{
			size_t len = strlen((const char*)utoa16(value, buf));
			if(len <= position)
			{
				const uint8_t offset = position + 2 - len;
				memmove(buf + offset, buf, len + 1);
				buf[0] = '0';
				buf[1] = '.';
				for(uint8_t x = 2; x < offset; ++x)
				{
					buf[x] = '0';
				}
			}
			else	//length > position
			{
				memmove(buf + len - position + 1, buf + len - position, position + 1);
				buf[len - position] = '.';
			}
			return buf;
		}
}//io


