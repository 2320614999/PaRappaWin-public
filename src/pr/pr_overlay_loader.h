#pragma once

#include "pr_scene.h"

struct PrGameContext;
struct PrSceneDef;

namespace PrScene1EntryOriginalDiscDirect {
struct Transaction80015D18;
}

class PrOverlayLoader {
public:
    bool Load(
        PrSceneId id,
        const PrSceneDef& def,
        PrGameContext& ctx,
        const PrScene1EntryOriginalDiscDirect::Transaction80015D18*
            scene1OriginalDiscTransaction = nullptr);
    void Unload(PrGameContext& ctx);

    bool IsLoaded() const { return m_loaded; }
    PrSceneId LoadedScene() const { return m_scene; }

private:
    bool m_loaded = false;
    PrSceneId m_scene = PrSceneId::Scene0;
};
