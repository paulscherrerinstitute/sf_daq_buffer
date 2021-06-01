
#include "include/PacketBuffer.hpp"


struct DummyContainer{
	uint64_t index;
	uint64_t timestamp;
	uint16_t data[32];
};


int main (int argc, char *argv[]) {
	PacketBuffer<DummyContainer, 64> b;

}
