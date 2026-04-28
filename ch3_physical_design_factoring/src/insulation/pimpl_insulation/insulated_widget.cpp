// insulated_widget.cpp
//
// All the heavy headers are confined HERE, never leaking to clients.

#ifndef INCLUDED_INSULATED_WIDGET
#include <insulated_widget.h>
#endif

// These heavy includes are HIDDEN from clients:
#include <algorithm>
#include <chrono>
#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

// ============================================================
// Implementation class — fully defined here, opaque to clients
// ============================================================

class InsulatedWidgetImpl {
  public:
    std::string                           d_name;
    std::vector<int>                      d_data;
    std::unordered_map<std::string, int>  d_lookup;
    std::set<std::string>                 d_tags;
    std::chrono::system_clock::time_point d_created;
    std::map<int, std::string>            d_history;

    explicit InsulatedWidgetImpl(const std::string& name)
    : d_name(name)
    , d_created(std::chrono::system_clock::now())
    {}
};


// ============================================================
// InsulatedWidget delegates to InsulatedWidgetImpl
// ============================================================

InsulatedWidget::InsulatedWidget(const std::string& name)
: d_impl_p(std::make_unique<InsulatedWidgetImpl>(name))
{}

InsulatedWidget::~InsulatedWidget() = default;
InsulatedWidget::InsulatedWidget(InsulatedWidget&&) noexcept = default;
InsulatedWidget& InsulatedWidget::operator=(InsulatedWidget&&) noexcept = default;

std::string InsulatedWidget::name() const { return d_impl_p->d_name; }

void InsulatedWidget::addData(int value)
{
    d_impl_p->d_data.push_back(value);
    d_impl_p->d_lookup[std::to_string(value)] = value;
}

int InsulatedWidget::dataSize() const
{
    return static_cast<int>(d_impl_p->d_data.size());
}

void InsulatedWidget::addTag(const std::string& tag)
{
    d_impl_p->d_tags.insert(tag);
}

bool InsulatedWidget::hasTag(const std::string& tag) const
{
    return d_impl_p->d_tags.count(tag) > 0;
}
