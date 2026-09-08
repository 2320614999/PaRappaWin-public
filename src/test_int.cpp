#include "int_loader.h"
#include <cstdio>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: test_int <INT_FILE>\n");
        printf("Example: test_int \"E:\\game\\PS模拟器\\parappa the rapper\\S0\\COMMON.INT\"\n");
        return 1;
    }
    
    const char* intPath = argv[1];
    printf("Loading: %s\n", intPath);
    
    IntArchive archive;
    if (!IntLoader::Load(intPath, archive)) {
        printf("ERROR: Failed to load INT file\n");
        return 1;
    }
    
    printf("Loaded %zu entries:\n\n", archive.entries.size());
    
    int timCount = 0, vabCount = 0, memCount = 0;
    
    for (const auto& entry : archive.entries) {
        const char* typeStr = "???";
        switch (entry.type) {
            case IntBlockType::Tim: typeStr = "TIM"; timCount++; break;
            case IntBlockType::Vab: typeStr = "VAB"; vabCount++; break;
            case IntBlockType::Mem: typeStr = "MEM"; memCount++; break;
            case IntBlockType::EoF: typeStr = "EOF"; break;
        }
        printf("  [%s] %-12s  offset=0x%08X  size=%u bytes\n",
               typeStr, entry.name.c_str(), entry.offset, (unsigned)entry.data.size());
    }
    
    printf("\nSummary: TIM=%d, VAB=%d, MEM=%d, Total=%zu\n",
           timCount, vabCount, memCount, archive.entries.size());
    
    return 0;
}
