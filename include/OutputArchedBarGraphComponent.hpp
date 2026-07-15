#ifndef OUTPUT_ARCHED_BAR_GRAPH_COMPONENT_HPP
#define OUTPUT_ARCHED_BAR_GRAPH_COMPONENT_HPP

#include "isobus/isobus/isobus_virtual_terminal_objects.hpp"
#include "isobus/isobus/isobus_virtual_terminal_server_managed_working_set.hpp"

#include "JuceHeader.h"

class OutputArchedBarGraphComponent : public isobus::OutputArchedBarGraph
  , public Component
{
public:
	OutputArchedBarGraphComponent(std::shared_ptr<isobus::VirtualTerminalServerManagedWorkingSet> workingSet, isobus::OutputArchedBarGraph sourceObject);

	void paint(Graphics &g) override;

private:
	float get_value_ratio(std::uint32_t value) const;
	float get_resolved_value_ratio() const;
	float get_resolved_target_ratio() const;
	float get_angle_for_ratio(float ratio, float startAngleRadians, float endAngleRadians) const;
	Point<float> get_point_on_ellipse(Rectangle<float> bounds, float angleRadians) const;
	void add_arc_to_path(Path &path, Rectangle<float> bounds, float fromRadians, float toRadians, bool startAsNewSubPath) const;
	Path create_bar_path(Rectangle<float> outerBounds, Rectangle<float> innerBounds, float fromRadians, float toRadians) const;
	void draw_radial_line(Graphics &g, Rectangle<float> outerBounds, Rectangle<float> innerBounds, float angleRadians, float lineWidth) const;

	std::shared_ptr<isobus::VirtualTerminalServerManagedWorkingSet> parentWorkingSet;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OutputArchedBarGraphComponent)
};

#endif // OUTPUT_ARCHED_BAR_GRAPH_COMPONENT_HPP
