#pragma once

#include <string>
#include <vector>
#include <exception>
#include <memory>
#include <boost/lexical_cast.hpp>

namespace TexData
{

class Node
{
public:
	/// Creates a name=value Node with a given parent. The line number is stored for debug purposes
	Node(std::string name, std::string value, Node* parent, int lineNumber = -1);
	Node(const Node& node);

	/// returns the Node of the newly created Child
	Node* AddChild(std::string name, std::string value, int lineNumber = -1);
	Node* GetParent();

	void DebugPrint(unsigned int indentation = 0);

	/// returns the i'th child of this Node
	Node& operator[] (unsigned int index);

	/// searches for a child with the given name and returns it
	Node& operator[] (std::string name);

	std::vector<std::unique_ptr<Node>>::iterator begin();
	std::vector<std::unique_ptr<Node>>::iterator end();

	/// returns the value of this node casted to type t
	template<typename t> t as()
	{
		if(m_Value.empty() && m_Children.empty())
			return boost::lexical_cast<t>(m_Name);
		else
			return boost::lexical_cast<t>(m_Value);
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