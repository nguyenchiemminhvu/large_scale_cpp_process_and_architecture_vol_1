// heavy_widget.cpp
#ifndef INCLUDED_HEAVY_WIDGET
#include <heavy_widget.h>
#endif

HeavyWidget::HeavyWidget(const std::string& name)
: d_name(name)
, d_created(std::chrono::system_clock::now())
{
}

std::string HeavyWidget::name() const { return d_name; }

void HeavyWidget::addData(int value)
{
    d_data.push_back(value);
    d_lookup[std::to_string(value)] = value;
}

int HeavyWidget::dataSize() const { return static_cast<int>(d_data.size()); }

void HeavyWidget::addTag(const std::string& tag) { d_tags.insert(tag); }

bool HeavyWidget::hasTag(const std::string& tag) const
{
    return d_tags.count(tag) > 0;
}
