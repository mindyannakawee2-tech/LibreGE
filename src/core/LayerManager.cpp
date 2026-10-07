#include "LayerManager.hpp"

#include <algorithm>

namespace LibreGE {

Layer& LayerManager::AddLayer(
    const std::string& name,
    int order
) {
    if (
        Layer* existing =
            GetLayer(name)
    ) {
        existing->SetOrder(
            order
        );

        m_Dirty = true;

        return *existing;
    }

    auto layer =
        std::make_unique<Layer>(
            name,
            order
        );

    Layer& reference =
        *layer;

    m_Layers.push_back(
        std::move(layer)
    );

    m_Dirty = true;

    return reference;
}

bool LayerManager::RemoveLayer(
    const std::string& name
) {
    const auto iterator =
        std::remove_if(
            m_Layers.begin(),
            m_Layers.end(),
            [&name](
                const std::unique_ptr<Layer>& layer
            ) {
                return
                    layer->GetName() ==
                    name;
            }
        );

    if (
        iterator ==
        m_Layers.end()
    ) {
        return false;
    }

    m_Layers.erase(
        iterator,
        m_Layers.end()
    );

    return true;
}

Layer* LayerManager::GetLayer(
    const std::string& name
) {
    for (
        auto& layer :
        m_Layers
    ) {
        if (
            layer->GetName() ==
            name
        ) {
            return layer.get();
        }
    }

    return nullptr;
}

const Layer* LayerManager::GetLayer(
    const std::string& name
) const {
    for (
        const auto& layer :
        m_Layers
    ) {
        if (
            layer->GetName() ==
            name
        ) {
            return layer.get();
        }
    }

    return nullptr;
}

void LayerManager::Sort() {
    if (!m_Dirty) {
        return;
    }

    std::stable_sort(
        m_Layers.begin(),
        m_Layers.end(),
        [](
            const std::unique_ptr<Layer>& a,
            const std::unique_ptr<Layer>& b
        ) {
            return
                a->GetOrder() <
                b->GetOrder();
        }
    );

    m_Dirty = false;
}

void LayerManager::ForEachOrdered(
    const std::function<void(Layer&)>& callback
) {
    Sort();

    for (
        auto& layer :
        m_Layers
    ) {
        if (
            !layer->IsEnabled()
        ) {
            continue;
        }

        callback(
            *layer
        );
    }
}

std::size_t
LayerManager::GetLayerCount() const {
    return m_Layers.size();
}

}
