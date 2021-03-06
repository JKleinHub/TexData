#pragma once

#include <string>
#include <vector>
#include <exception>
#include <memory>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>// for Node::split

namespace TexData
{

	inline boost::algorithm::detail::is_any_ofF<char> whitespaces()
	{
		return boost::is_any_of(" \n\t\v\f\r");
	}

	inline std::string nameValueSeparator()
	{
		return "=";
	}

class Node
{
public:
	/// Creates a name=value Node with a given parent. The line number is stored for debug purposes
	Node(std::string name, std::string value, Node* parent, int lineNumber = -1);
	Node(const Node& node);

	std::string Name() const { return m_Name; }
	std::string Value() const { return m_Value; }

	/// returns the Node of the newly created Child
	Node* AddChild(std::string name, std::string value, int lineNumber = -1);
	Node* GetParent() const;

	void DebugPrint(unsigned int indentation = 0) const;

	/// returns the i'th child of this Node
	const Node& operator[] (unsigned int index) const;

	/// searches for a child with the given name and returns it
	const Node& operator[] (std::string name) const;

	/// checks, whether a child with the given name exists
	/** This will only return the first child with the given name. If you have
		multiple elements with the same name, use the iterator interface or
		other operator[] and filter them yourself.
	**/
	bool has(std::string name) const;

	std::vector<std::unique_ptr<Node>>::const_iterator begin() const;
	std::vector<std::unique_ptr<Node>>::const_iterator end() const;

	/// returns the value of this node casted to type t
	template<typename t> t as() const
	{
		try
		{
			if(m_Value.empty() && m_Children.empty())
				return boost::lexical_cast<t>(m_Name);
			else
				return boost::lexical_cast<t>(m_Value);
		}
		catch(std::exception& ex)//TODO add more information to this exception...
		{
			throw Exception(ex.what());
		}
	}

	/// splits a line in a vector of primitives of the same type, all whitespaces between separators will be trimmed
	template<typename t> std::vector<t> split(const char* split=",") const
	{
		std::vector<std::string> splitted;
		boost::split(splitted, m_Value, boost::is_any_of(split));
		std::vector<t> result;
		for(auto& s : splitted)
			result.push_back(boost::lexical_cast<t>(boost::trim_copy_if(s, whitespaces())));
		return result;
	}
private:
	/// returns a string describing the path to the root
	std::string PathToRoot();

	std::string m_Name;
	std::string m_Value;
	std::vector<std::unique_ptr<Node>> m_Children;
	Node* m_Parent = nullptr;

	/// line in which this node was defined (-1 is interpreted as 'not set')
	int m_LineNumber;
};

Node LoadFile(std::string filename);
Node LoadString(std::string inputString);



class Exception : public std::exception
{
public:
	Exception(std::string errorSummary);
	void SetFilename(std::string filename);
	void SetBadLine(std::string badLine);

	const char* what() const override;
private:
	std::string m_Summary;
	std::string m_BadLine;
	std::string m_Filename;

	mutable std::string m_Message;
};

}