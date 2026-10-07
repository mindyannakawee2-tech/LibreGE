#pragma once

#include <string>

namespace LibreGE {

class Layer {
public:
    Layer(
        std::string name,
        int order
    );

    const std::string& GetName() const;

    int GetOrder() const;

    void SetOrder(
        int order
    );

    bool IsEnabled() const;

    void SetEnabled(
        bool enabled
    );

private:
    std::string m_Name;

    int m_Order = 0;

    bool m_Enabled = true;
};

}
