#pragma once

#include <functional>
#include <string>
#include <map>

/*
simple "reflection" (or "introspection") for class members
allows to bind a string "tag" ("field") to a class member with getter and setter callbacks for this member
usage: see ConsoleFrameViewOptionsReflection in Command.*
*/

template<typename Class>
struct ClassReflection
{
	using getter = std::function<void()>;  // really it's not truly "getter", but "printer"
	using setter = std::function<void(const std::string&)>;

	explicit ClassReflection(Class& obj)
		: m_obj(obj)
	{}

	void get(const std::string& field)
	{
		if (m_reflection.find(field) != m_reflection.end())
			(m_reflection[field].first)();
		else
			throw std::runtime_error("unknown argument: " + field);
	}

	void set(const std::string& field, const std::string& value)
	{
		if (m_reflection.find(field) != m_reflection.end())
			(m_reflection[field].second)(value);
		else
			throw std::runtime_error("unknown argument: " + field);
	}

	void get_all()
	{
		for (auto& [field, callbacks] : m_reflection) 
		{ 
			get(field); 
		}
	}

protected:
	Class& m_obj;
	std::map<std::string, std::pair<getter, setter>> m_reflection;
};
