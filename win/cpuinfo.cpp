#include <windows.h>
#include <malloc.h>
#include <stdio.h>

struct CpuInfo {
	int coreNum;
	enum Type {
		unified = 0,
		code = 1,
		data = 2
	};
	int cacheSize[3][3];
	CpuInfo() : coreNum(0), cacheSize{} {}
	void put() const
	{
		printf("coreNum=%d\n", coreNum);
		for (int level = 0; level < 3; level++) {
			for (int type = 0; type < 3; type++) {
				const char *s = (type == 0) ? "unified" : (type == 1) ? "code" : "data";
				printf("L%d %s size = %d\n", level + 1, s, cacheSize[type][level]);
			}
		}
	}
	void init()
	{
		DWORD bufSize = 0;
		GetLogicalProcessorInformation (NULL, &bufSize);
		auto *ptr = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION)_alloca(bufSize);
		if (GetLogicalProcessorInformation (ptr, &bufSize) == FALSE) return;

		DWORD offset = 0;
		while (offset + sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION) <= bufSize) {
			switch (ptr->Relationship) {
			case RelationProcessorCore:
				coreNum++;
				break;

			case RelationCache:
				{
					const auto cache = &ptr->Cache;
					if (1 <= cache->Level && cache->Level <= 3) {
						cacheSize[cache->Type][cache->Level - 1] += cache->Size;
					}
				}
				break;
			default:
				break;
			}
			offset += sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);
			ptr++;
		}
	}
};

int main()
{
	CpuInfo cpuInfo;
	cpuInfo.init();

	cpuInfo.put();
}