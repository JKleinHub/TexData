#include "TexData.hpp"

#include <fstream>
#include <sstream>
#include <iostream>
#include <boost/algorithm/string.hpp>
#include <algorithm>

using std::ifstream;
using std::string;
using std::vector;
using std::getline;
using std::make_unique;
using std::unique_ptr;
using std::find_if;

using std::cout;
using std::endl;


namespace TexData
{

Node::Node(string name, string value, Node* parent, int lineNumber) :
m_Name(name),
m_Value(value),
m_Parent(parent),
m_LineNumber(lineNumber)
{

}

Node::Node(const Node& node):
m_Name(node.m_Name),
m_Value(node.m_Value),
m_Parent(node.m_Parent),
m_LineNumber(node.m_LineNumber)
{
	//copy all children
	for(auto& c : node.m_Children)
		m_Children.push_back(make_unique<Node>(*c.get()));
}

Node* Node::AddChild(string name, string value, int lineNumber)
{
	m_Children.push_back(make_unique<Node>(name, value, this, lineNumber));// create the child
	return m_Children.rbegin()->get(); // and return its pointer
}

Node* Node::GetParent()
{
	return m_Parent;
}

void Node::DebugPrint(unsigned int indentation)
{
	for(auto i = 0u; i<indentation; ++i)
		cout << "  ";
	if(!m_Name.empty())
		cout << m_Name << ": ";
	cout << m_Value << endl;
	for(auto& c : m_Children)
	{
		c->DebugPrint(indentation + 1);
	}
}

Node& Node::operator[](unsigned int index)
{
	return *(m_Children[index].get());
}

Node& Node::operator[](string name)
{
	auto it = find_if(m_Children.begin(), m_Children.end(),
		[&name](unique_ptr<Node>& c )
		{
			return c->m_Name == name;
		});
	if(m_Children.end() == it)
	{
		auto errorMessage = name+" not found in Node "+m_Name;
		if(-1 != m_LineNumber)
			errorMessage += " (Line "+std::to_string(m_LineNumber)+")";
		throw Exception(errorMessage);
	}
	else
		return *(it->get());
}

bool Node::has(string name)
{
	auto it = find_if(m_Children.begin(), m_Children.end(),
		[&name](unique_ptr<Node>& c)
	{
		return c->m_Name == name;
	});
	return it!=m_Children.end();
}

vector<unique_ptr<Node>>::iterator Node::begin()
{
	return m_Children.begin();
}

vector<unique_ptr<Node>>::iterator Node::end()
{
	return m_Children.end();
}

template<typename t_stream> Node Load(t_stream& stream)
{
	Node rootNode("root", "", nullptr);
	Node* lastNode = nullptr;
	Node* currentNode = &rootNode;
	unsigned int lastIndentation = 0;

	// Parse files
	auto lineCounter = 0u; //Line counter for Exceptions
	for(string line; getline(stream, line);)
	{
		try
		{
			lineCounter++;

			// check for empty lines
			if(boost::trim_copy_if(line, whitespaces()).empty())
				continue;

			// ------- split name = value -------
			string name, value;
			vector<string> splittedLine;
			boost::split(splittedLine, line, boost::is_any_of(nameValueSeparator()));
			if(1 == splittedLine.size())
			{
				name = boost::trim_copy_if(splittedLine[0], whitespaces());
			}
			else if(2 == splittedLine.size())
			{
				name = boost::trim_copy_if(splittedLine[0], whitespaces());
				value = boost::trim_copy_if(splittedLine[1], whitespaces());
				if(value.empty())
					throw Exception("empty value");
			}
			else
			{
				throw Exception(string("too many '")+nameValueSeparator()+"'");
			}

			// ------- determine indentation -------
			const auto indentationCharacter = '\t';
			auto const contentStart = find_if_not(
				line.begin(), line.end(),
				[](const char c){ return c=='\t'; });
			const unsigned int currentIndentation = std::distance(line.begin(), contentStart);


			// ------- insert new element -------
			if(currentIndentation == lastIndentation)
			{
				//no need to do anything
			}
			else if(currentIndentation == lastIndentation + 1)
			{
				currentNode = lastNode;
			}
			else if(currentIndentation < lastIndentation)
			{
				for(auto i = 0u; i< lastIndentation - currentIndentation; ++i)
					currentNode = currentNode->GetParent();
			}
			else // new indentation is more than 2 bigger than old one
			{
				throw Exception("Indentation too deep");
			}
			lastNode = currentNode->AddChild(name, value, lineCounter);

			lastIndentation = currentIndentation;
		}
		catch(Exception& e)
		{
			e.SetBadLine(std::to_string(lineCounter) + ": \"" + boost::trim_copy_if(line, whitespaces()) + "\"");
			throw;
		}
	}

	return rootNode;
}

Node LoadFile(string filename)
{
	try
	{
		ifstream file(filename);
		if(!file.is_open())
			throw Exception("File not found");
		return Load(file);
	}
	catch(Exception& e)
	{
		e.SetFilename(filename);
		throw;
	}
}

Node LoadString(std::string inputString)
{
	std::stringstream stream;
	stream << inputString;
	return Load(stream);
}

Exception::Exception(string errorSummary):
m_Summary(errorSummary)
{

}

const char* Exception::what() const
{
	m_Message = "TexData Error: " + m_Summary;
	if(!m_BadLine.empty())
		m_Message += " in line " + m_BadLine;
	if(!m_Filename.empty())
		m_Message += ", while loading " + m_Filename;
	return m_Message.c_str();
}

void Exception::SetFilename(string filename)
{
	m_Filename = filename;
}

void Exception::SetBadLine(string badLine)
{
	m_BadLine = badLine;
}


}
