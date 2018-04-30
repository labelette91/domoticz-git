#include "stdafx.h"
#include "../main/Helper.h"
#include "hardwaretypes.h"
#include "ColorSwitch.h"

_tColor::_tColor()
{
	//level = 0;
	t = r = g = b = cw = ww = 0;
	mode = ColorModeNone;
}

_tColor::_tColor(Json::Value json)
{
	fromJSON(json);
}

_tColor::_tColor(const std::string sRaw) //explicit to avoid unintentional conversion of string to _tColor
{
	fromString(sRaw);
}

_tColor::_tColor(const uint8_t ir, const uint8_t ig, const uint8_t ib, const uint8_t icw, const uint8_t iww, ColorMode imode)
{
	mode = imode;
	//level=ilevel;
	r=ir; g=ig; b=ib; cw=icw; ww=iww;
}

_tColor::_tColor(uint8_t x, ColorMode imode)
{
	_tColor();
	if (imode == ColorModeWhite)
	{
		mode = imode;
		ww = 0xff;
		cw = 0xff;
	}
	else if (imode == ColorModeTemp)
	{
		mode = imode;
		ww = x;
		cw = 255-x;
		t = x;
	}
}

std::string _tColor::getrgbwwhex() const
{
	char tmp[13];
	snprintf(tmp, sizeof(tmp), "%02x%02x%02x%02x%02x", r, g, b, cw, ww);
	return std::string(tmp);
}

void _tColor::fromJSON(Json::Value root)
{
	mode = ColorModeNone;
	int tmp;
	try {
		tmp = root.get("m", 0).asInt();
		if (tmp == ColorModeNone || tmp > ColorModeLast) return;
		mode = ColorMode(tmp);
		t = root.get("t", 0).asInt();
		r = root.get("r", 0).asInt();
		g = root.get("g", 0).asInt();
		b = root.get("b", 0).asInt();
		cw = root.get("cw", 0).asInt();
		ww = root.get("ww", 0).asInt();
		//level = root.get("l", 0).asInt();
	}
	catch (...) {
	}
}

void _tColor::fromString(std::string s)
{
	Json::Value root;
	Json::Reader reader(Json::Features::strictMode());
	mode = ColorModeNone;
	try {
		bool parsingSuccessful = reader.parse(s.c_str(), root);     //parse process
		if ( !parsingSuccessful )
		{
			mode = ColorModeNone;
			return;
		}
		fromJSON(root);
	}
	catch (...) {
	}
}

std::string _tColor::toJSON() const
{
	// Return the empty string if the color is not valid
	if (mode == ColorModeNone || mode > ColorModeLast) return "";

	Json::Value root;
	root["m"] = mode;
	//root["l"] = level;
	root["t"] = t;
	root["r"] = r;
	root["g"] = g;
	root["b"] = b;
	root["cw"] = cw;
	root["ww"] = ww;

	Json::FastWriter fastwriter;
	fastwriter.omitEndingLineFeed();
	return fastwriter.write(root);
}

std::string _tColor::toString() const
{
	char tmp[1024];
	// Return the empty string if the color is not valid
	if (mode == ColorModeNone || mode > ColorModeLast) return "{INVALID}";

	snprintf(tmp, sizeof(tmp), "{m: %u, RGB: %02x%02x%02x, CWWW: %02x%02x, CT: %u}", mode, r, g, b, cw, ww, t);

	return std::string(tmp);
}
