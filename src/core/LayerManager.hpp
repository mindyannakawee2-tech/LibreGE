#pragma once

#include "Layer.hpp"

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace LibreGE {

class LayerManager {
public:
    Layer& AddLayer(
        const std::string& name,
        int order
    );

    bool RemoveLayer(
        const std::string& name
    );

    Layer* GetLayer(
        const std::string& name
    );

    const Layer* GetLayer(
        const std::string& name
    ) const;

    void Sort();

    void ForEachOrdered(
        const std::function<void(Layer&)>& callback
    );

    std::size_t GetLayerCount() const;

private:
    std::vector<
        std::unique_ptr<Layer>
    > m_Layers;

    bool m_Dirty = false;
};

}
