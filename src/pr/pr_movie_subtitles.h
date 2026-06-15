#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace PrMovieSubtitles {

struct MovieSubtitleLine {
    uint32_t frame30 = 0;
    uint16_t duration = 0;
    int16_t textIndex[5]{};
    std::string texts[5];
};

struct MovieSubtitleTrack {
    bool loaded = false;
    uint32_t descOffset = 0;
    uint32_t entryCount = 0;
    std::vector<MovieSubtitleLine> lines;
};

}
