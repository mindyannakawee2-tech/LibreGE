#include "Layer.hpp"

#include <utility>

namespace LibreGE {

Layer::Layer(
    std::string name,
    int order
)
    : m_Name(
        std::move(name)
      ),
      m_Order(order) {
}

const std::string&
Layer::GetName() const {
    return m_Name;
}

int Layer::GetOrder() const {
    return m_Order;
}

void Layer::SetOrder(
    int order
) {
    m_Order = order;
}

bool Layer::IsEnabled() const {
    return m_Enabled;
}

void Layer::SetEnabled(
    bool enabled
) {
    m_Enabled = enabled;
}

}
