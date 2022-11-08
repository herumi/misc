#include <windows.h>
#include <malloc.h>
#include <stdio.h>
#include <stdint.h>

struct CpuInfo {
	int coreNum;
	enum Type {
		Unified = 0,
		Code = 1,
		Data = 2
	};
	int cacheSize[3][3];
	CpuInfo() : coreNum(0), cacheSize{} {}
	int getCacheSize(Type type, uint32_t level) const
	{
		if (1 <= level && level <= 3) return cacheSize[type][level - 1];
		return 0;
	}
	int getUnifiedCacheSize(int level) const { return getCacheSize(Unified, level); }
	int getCodeCacheSize(int level) const { return getCacheSize(Code, level); }
	int getDataCacheSize(int level) const { return getCacheSize(Data, level); }
	void put() const
	{
		printf("coreNum=%d\n", coreNum);
		for (int level = 1; level <= 3; level++) {
			printf("L%d unified size = %d\n", level, getUnifiedCacheSize(level));
			printf("L%d code size = %d\n", level, getCodeCacheSize(level));
			printf("L%d data size = %d\n", level, getDataCacheSize(level));
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