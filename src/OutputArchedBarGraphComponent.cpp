/*******************************************************************************
** @file       OutputArchedBarGraphComponent.cpp
** @copyright  The Open-Agriculture Developers
*******************************************************************************/
#include "OutputArchedBarGraphComponent.hpp"

#include <algorithm>
#include <cmath>

OutputArchedBarGraphComponent::OutputArchedBarGraphComponent(std::shared_ptr<isobus::VirtualTerminalServerManagedWorkingSet> workingSet, isobus::OutputArchedBarGraph sourceObject) :
  isobus::OutputArchedBarGraph(sourceObject),
  parentWorkingSet(workingSet)
{
	setSize(get_width(), get_height());
	setOpaque(false);
}

void OutputArchedBarGraphComponent::paint(Graphics &g)
{
	if ((0 == get_width()) || (0 == get_height()))
	{
		return;
	}

	const Rectangle<float> outerBounds = getLocalBounds().toFloat().reduced(0.5f);
	const float maximumBarWidth = std::min(outerBounds.getWidth(), outerBounds.getHeight()) / 2.0f;
	const float barWidth = std::min(static_cast<float>(get_bar_graph_width()), maximumBarWidth);

	if (barWidth <= 0.0f)
	{
		return;
	}

	Rectangle<float> innerBounds = outerBounds.reduced(barWidth);
	if ((innerBounds.getWidth() < 0.0f) || (innerBounds.getHeight() < 0.0f))
	{
		innerBounds = Rectangle<float>(outerBounds.getCentreX(), outerBounds.getCentreY(), 0.0f, 0.0f);
	}

	const auto vtBarColour = parentWorkingSet->get_colour(get_colour());
	const auto barColour = Colour::fromFloatRGBA(vtBarColour.r, vtBarColour.g, vtBarColour.b, 1.0f);
	const auto vtTargetLineColour = parentWorkingSet->get_colour(get_target_line_colour());
	const auto targetLineColour = Colour::fromFloatRGBA(vtTargetLineColour.r, vtTargetLineColour.g, vtTargetLineColour.b, 1.0f);

	constexpr float twoPi = juce::MathConstants<float>::twoPi;
	constexpr float angleEpsilon = 0.0001f;
	const float rawStartAngle = std::fmod(juce::degreesToRadians(static_cast<float>(get_start_angle()) * 2.0f), twoPi);
	const float rawEndAngle = std::fmod(juce::degreesToRadians(static_cast<float>(get_end_angle()) * 2.0f), twoPi);
	// The arched bar scale runs from the object's end angle toward its start angle.
	float arcStartAngle = rawEndAngle;
	float arcEndAngle = rawStartAngle;

	if (arcStartAngle < 0.0f)
	{
		arcStartAngle += twoPi;
	}
	if (arcEndAngle < 0.0f)
	{
		arcEndAngle += twoPi;
	}

	const bool clockwise = get_option(Options::Deflection);
	const bool sameAngle = std::fabs(arcStartAngle - arcEndAngle) < angleEpsilon;

	if (clockwise)
	{
		if (sameAngle)
		{
			arcEndAngle = arcStartAngle - twoPi;
		}
		else if (arcEndAngle > arcStartAngle)
		{
			arcEndAngle -= twoPi;
		}
	}
	else
	{
		if (sameAngle)
		{
			arcEndAngle = arcStartAngle + twoPi;
		}
		else if (arcEndAngle < arcStartAngle)
		{
			arcEndAngle += twoPi;
		}
	}

	const float valueAngle = get_angle_for_ratio(get_resolved_value_ratio(), arcStartAngle, arcEndAngle);

	g.setColour(barColour);
	if (get_option(Options::BarGraphType))
	{
		draw_radial_line(g, outerBounds, innerBounds, valueAngle, 3.0f);
	}
	else
	{
		g.fillPath(create_bar_path(outerBounds, innerBounds, arcStartAngle, valueAngle));
	}

	if (get_option(Options::DrawBorder))
	{
		Path outerBorder;
		add_arc_to_path(outerBorder, outerBounds, arcStartAngle, arcEndAngle, true);
		g.strokePath(outerBorder, PathStrokeType(1.0f));

		if ((innerBounds.getWidth() > 0.0f) && (innerBounds.getHeight() > 0.0f))
		{
			Path innerBorder;
			add_arc_to_path(innerBorder, innerBounds, arcStartAngle, arcEndAngle, true);
			g.strokePath(innerBorder, PathStrokeType(1.0f));
		}

		if (!sameAngle)
		{
			draw_radial_line(g, outerBounds, innerBounds, arcStartAngle, 1.0f);
			draw_radial_line(g, outerBounds, innerBounds, arcEndAngle, 1.0f);
		}
	}

	if (get_option(Options::DrawTargetLine))
	{
		g.setColour(targetLineColour);
		draw_radial_line(g, outerBounds, innerBounds, get_angle_for_ratio(get_resolved_target_ratio(), arcStartAngle, arcEndAngle), 1.0f);
	}
}

float OutputArchedBarGraphComponent::get_value_ratio(std::uint32_t value) const
{
	const auto minValue = static_cast<std::uint32_t>(get_min_value());
	const auto maxValue = static_cast<std::uint32_t>(get_max_value());

	if (maxValue <= minValue)
	{
		return 0.0f;
	}

	const auto clampedValue = std::min(maxValue, std::max(minValue, value));
	return static_cast<float>(clampedValue - minValue) / static_cast<float>(maxValue - minValue);
}

float OutputArchedBarGraphComponent::get_resolved_value_ratio() const
{
	std::uint32_t value = get_value();

	if (isobus::NULL_OBJECT_ID != get_variable_reference())
	{
		auto child = get_object_by_id(get_variable_reference(), parentWorkingSet->get_object_tree());

		if ((nullptr != child) && (isobus::VirtualTerminalObjectType::NumberVariable == child->get_object_type()))
		{
			value = std::static_pointer_cast<isobus::NumberVariable>(child)->get_value();
		}
	}
	return get_value_ratio(value);
}

float OutputArchedBarGraphComponent::get_resolved_target_ratio() const
{
	std::uint32_t value = get_target_value();

	if (isobus::NULL_OBJECT_ID != get_target_value_reference())
	{
		auto child = get_object_by_id(get_target_value_reference(), parentWorkingSet->get_object_tree());

		if ((nullptr != child) && (isobus::VirtualTerminalObjectType::NumberVariable == child->get_object_type()))
		{
			value = std::static_pointer_cast<isobus::NumberVariable>(child)->get_value();
		}
	}
	return get_value_ratio(value);
}

float OutputArchedBarGraphComponent::get_angle_for_ratio(float ratio, float startAngleRadians, float endAngleRadians) const
{
	return startAngleRadians + ((endAngleRadians - startAngleRadians) * ratio);
}

Point<float> OutputArchedBarGraphComponent::get_point_on_ellipse(Rectangle<float> bounds, float angleRadians) const
{
	constexpr float halfPi = juce::MathConstants<float>::halfPi;
	constexpr float twoPi = juce::MathConstants<float>::twoPi;
	constexpr float angleEpsilon = 1e-6f;
	const auto halfWidth = bounds.getWidth() / 2.0f;
	const auto halfHeight = bounds.getHeight() / 2.0f;

	if ((halfWidth <= 0.0f) || (halfHeight <= 0.0f))
	{
		return bounds.getCentre();
	}

	auto angle = std::fmod(angleRadians, twoPi);
	if (angle < 0.0f)
	{
		angle += twoPi;
	}

	double x = 0.0;
	double y = 0.0;

	if (std::fabs(angle - halfPi) < angleEpsilon)
	{
		y = -halfHeight;
	}
	else if (std::fabs(angle - (3.0f * halfPi)) < angleEpsilon)
	{
		y = halfHeight;
	}
	else
	{
		const double tanAngle = std::tan(angle);
		const double div = std::sqrt((halfHeight * halfHeight) + (halfWidth * halfWidth) * (tanAngle * tanAngle));
		x = (halfWidth * halfHeight) / div;
		y = -(halfWidth * halfHeight * tanAngle) / div;

		if (!((angle < halfPi) || (angle > (3.0f * halfPi))))
		{
			x = -x;
			y = -y;
		}
	}

	return { static_cast<float>(bounds.getCentreX() + x), static_cast<float>(bounds.getCentreY() + y) };
}

void OutputArchedBarGraphComponent::add_arc_to_path(Path &path, Rectangle<float> bounds, float fromRadians, float toRadians, bool startAsNewSubPath) const
{
	const float direction = (toRadians >= fromRadians) ? 1.0f : -1.0f;
	const float step = 0.05f * direction;
	bool firstPoint = true;

	auto addPoint = [&](float angle) {
		const auto point = get_point_on_ellipse(bounds, angle);

		if (firstPoint && startAsNewSubPath)
		{
			path.startNewSubPath(point);
		}
		else
		{
			path.lineTo(point);
		}
		firstPoint = false;
	};

	for (float angle = fromRadians; (direction > 0.0f) ? (angle < toRadians) : (angle > toRadians); angle += step)
	{
		addPoint(angle);
	}
	addPoint(toRadians);
}

Path OutputArchedBarGraphComponent::create_bar_path(Rectangle<float> outerBounds, Rectangle<float> innerBounds, float fromRadians, float toRadians) const
{
	Path barPath;
	add_arc_to_path(barPath, outerBounds, fromRadians, toRadians, true);
	add_arc_to_path(barPath, innerBounds, toRadians, fromRadians, false);
	barPath.closeSubPath();
	return barPath;
}

void OutputArchedBarGraphComponent::draw_radial_line(Graphics &g, Rectangle<float> outerBounds, Rectangle<float> innerBounds, float angleRadians, float lineWidth) const
{
	const auto outerPoint = get_point_on_ellipse(outerBounds, angleRadians);
	const auto innerPoint = get_point_on_ellipse(innerBounds, angleRadians);
	g.drawLine(outerPoint.x, outerPoint.y, innerPoint.x, innerPoint.y, lineWidth);
}
