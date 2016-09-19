#include "typeconverters.h"

bool conv_rect_to_string(const nap::Rect &inval, std::string &outval)
{
	std::ostringstream ss;
	ss << inval.getX();
	ss << ",";
	ss << inval.getY();
	ss << ",";
	ss << inval.getWidth();
	ss << ",";
	ss << inval.getHeight();
	outval = ss.str();
	return true;
}

bool conv_string_to_rect(const std::string& inval, nap::Rect& outval)
{
	std::list<std::string> tokens;
	nap::gTokenize(inval, tokens, ",", true);
	if (tokens.size() == 4) {
		outval.setX((float)std::atof(tokens.front().c_str()));
		tokens.pop_front();
		outval.setY((float)std::atof(tokens.front().c_str()));
		tokens.pop_front();
		outval.setWidth((float)std::atof(tokens.front().c_str()));
		tokens.pop_front();
		outval.setHeight((float)std::atof(tokens.front().c_str()));
		tokens.pop_front();
		return true;
	}
	return false;
}

bool conv_margins_to_string(const nap::Margins& inval, std::string& outval)
{
	std::ostringstream ss;
	ss << inval.getLeft();
	ss << ",";
	ss << inval.getTop();
	ss << ",";
	ss << inval.getRight();
	ss << ",";
	ss << inval.getBottom();
	outval = ss.str();
	return true;
}

bool conv_string_to_margins(const std::string& inval, nap::Margins& outval)
{
	std::list<std::string> tokens;
	nap::gTokenize(inval, tokens, ",");
	if (tokens.size() == 4) {
		outval.setLeft((float)std::atof(tokens.front().c_str()));
		tokens.pop_front();
		outval.setTop((float)std::atof(tokens.front().c_str()));
		tokens.pop_front();
		outval.setRight((float)std::atof(tokens.front().c_str()));
		tokens.pop_front();
		outval.setBottom((float)std::atof(tokens.front().c_str()));
		tokens.pop_front();
		return true;
	}
	return false;
}
