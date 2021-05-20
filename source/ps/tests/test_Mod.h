/* Copyright (C) 2021 Wildfire Games.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "lib/self_test.h"

#include "ps/CLogger.h"
#include "ps/Mod.h"
#include "scriptinterface/JSON.h"
#include "scriptinterface/ScriptInterface.h"

class TestMod : public CxxTest::TestSuite
{
public:
	void test_version_check()
	{
		CStr eq = "=";
		CStr lt = "<";
		CStr gt = ">";
		CStr leq = "<=";
		CStr geq = ">=";

		CStr required = "0.0.24";// 0ad <= required
		CStr version = "0.0.24";// 0ad version

		// 0.0.24 = 0.0.24
		TS_ASSERT(g_Mods.CompareVersionStrings(version, eq, required));
		TS_ASSERT(!g_Mods.CompareVersionStrings(version, lt, required));
		TS_ASSERT(!g_Mods.CompareVersionStrings(version, gt, required));
		TS_ASSERT(g_Mods.CompareVersionStrings(version, leq, required));
		TS_ASSERT(g_Mods.CompareVersionStrings(version, geq, required));

		// 0.0.23 <= 0.0.24
		version = "0.0.23";
		TS_ASSERT(!g_Mods.CompareVersionStrings(version, eq, required));
		TS_ASSERT(g_Mods.CompareVersionStrings(version, lt, required));
		TS_ASSERT(!g_Mods.CompareVersionStrings(version, gt, required));
		TS_ASSERT(g_Mods.CompareVersionStrings(version, leq, required));
		TS_ASSERT(!g_Mods.CompareVersionStrings(version, geq, required));

		// 0.0.25 >= 0.0.24
		version = "0.0.25";
		TS_ASSERT(!g_Mods.CompareVersionStrings(version, eq, required));
		TS_ASSERT(!g_Mods.CompareVersionStrings(version, lt, required));
		TS_ASSERT(g_Mods.CompareVersionStrings(version, gt, required));
		TS_ASSERT(!g_Mods.CompareVersionStrings(version, leq, required));
		TS_ASSERT(g_Mods.CompareVersionStrings(version, geq, required));

		// 0.0.9 <= 0.1.0
		version = "0.0.9";
		required = "0.1.0";
		TS_ASSERT(!g_Mods.CompareVersionStrings(version, eq, required));
		TS_ASSERT(g_Mods.CompareVersionStrings(version, lt, required));
		TS_ASSERT(!g_Mods.CompareVersionStrings(version, gt, required));
		TS_ASSERT(g_Mods.CompareVersionStrings(version, leq, required));
		TS_ASSERT(!g_Mods.CompareVersionStrings(version, geq, required));

		// 5.3 <= 5.3.0
		version = "5.3";
		required = "5.3.0";
		TS_ASSERT(!g_Mods.CompareVersionStrings(version, eq, required));
		TS_ASSERT(g_Mods.CompareVersionStrings(version, lt, required));
		TS_ASSERT(!g_Mods.CompareVersionStrings(version, gt, required));
		TS_ASSERT(g_Mods.CompareVersionStrings(version, leq, required));
		TS_ASSERT(!g_Mods.CompareVersionStrings(version, geq, required));
	}

	void test_compatible()
	{
		ScriptInterface script("Test", "Test", g_ScriptContext);

		ScriptRequest rq(script);
		JS::RootedObject obj(rq.cx, JS_NewPlainObject(rq.cx));

		CStr jsonString = "{\
				\"name\": \"0ad\",\
				\"version\" : \"0.0.25\",\
				\"label\" : \"0 A.D. Empires Ascendant\",\
				\"url\" : \"https://play0ad.com\",\
				\"description\" : \"A free, open-source, historical RTS game.\",\
				\"dependencies\" : []\
				}\
			";
		JS::RootedValue json(rq.cx);
		TS_ASSERT(Script::ParseJSON(rq, jsonString, &json));
		JS_SetProperty(rq.cx, obj, "public", json);

		JS::RootedValue jsonW(rq.cx);
		CStr jsonStringW = "{\
				\"name\": \"wrong\",\
				\"version\" : \"0.0.25\",\
				\"label\" : \"wrong mod\",\
				\"url\" : \"\",\
				\"description\" : \"fail\",\
				\"dependencies\" : [\"0ad=0.0.24\"]\
				}\
			";
		TS_ASSERT(Script::ParseJSON(rq, jsonStringW, &jsonW));
		JS_SetProperty(rq.cx, obj, "wrong", jsonW);

		JS::RootedValue jsonG(rq.cx);
		CStr jsonStringG = "{\
				\"name\": \"good\",\
				\"version\" : \"0.0.25\",\
				\"label\" : \"good mod\",\
				\"url\" : \"\",\
				\"description\" : \"ok\",\
				\"dependencies\" : [\"0ad=0.0.25\"]\
				}\
			";
		TS_ASSERT(Script::ParseJSON(rq, jsonStringG, &jsonG));
		JS_SetProperty(rq.cx, obj, "good", jsonG);

		JS::RootedValue jsonG2(rq.cx);
		CStr jsonStringG2 = "{\
				\"name\": \"good\",\
				\"version\" : \"0.0.25\",\
				\"label\" : \"good mod\",\
				\"url\" : \"\",\
				\"description\" : \"ok\",\
				\"dependencies\" : [\"0ad>=0.0.24\"]\
				}\
			";
		TS_ASSERT(Script::ParseJSON(rq, jsonStringG2, &jsonG2));
		JS_SetProperty(rq.cx, obj, "good2", jsonG2);

		JS::RootedValue availableMods(rq.cx, JS::ObjectValue(*obj));

		std::vector<CStr> mods;

		mods.clear();
		mods.push_back("public");
		g_Mods.ClearIncompatibleMods();
		TS_ASSERT(g_Mods.AreModsCompatible(script, mods, availableMods));

		mods.clear();
		mods.push_back("mod");
		mods.push_back("public");
		g_Mods.ClearIncompatibleMods();
		TS_ASSERT(g_Mods.AreModsCompatible(script, mods, availableMods));

		mods.clear();
		mods.push_back("public");
		mods.push_back("good");
		g_Mods.ClearIncompatibleMods();
		TS_ASSERT(g_Mods.AreModsCompatible(script, mods, availableMods));

		mods.clear();
		mods.push_back("public");
		mods.push_back("good2");
		g_Mods.ClearIncompatibleMods();
		TS_ASSERT(g_Mods.AreModsCompatible(script, mods, availableMods));

		mods.clear();
		mods.push_back("public");
		mods.push_back("wrong");
		g_Mods.ClearIncompatibleMods();
		TS_ASSERT(!g_Mods.AreModsCompatible(script, mods, availableMods));

		mods.clear();
		mods.push_back("public");
		mods.push_back("does_not_exist");
		g_Mods.ClearIncompatibleMods();
		TS_ASSERT(!g_Mods.AreModsCompatible(script, mods, availableMods));

	}
};
