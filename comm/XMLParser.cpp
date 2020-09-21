#include "stdafx.h"
#include <cassert>
#include <fstream>
#include "XMLParser.h"

//namespace EUI_XML_PARSER
//{

///////////////////////////////////////////////////////////////////////////////////////////////////
inline XmlBase::XmlBase()
	: m_name(T(""))
	, m_value(T(""))
	, m_nameAllocated(false)
	, m_valueAllocated(false)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline XmlBase::~XmlBase()
{
	if (m_nameAllocated && m_name != NULL)
	{
		delete m_name;
	}
	if (m_valueAllocated && m_value != NULL)
	{
		delete m_value;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline const Char* XmlBase::getName() const
{
	return m_name;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline void XmlBase::setName(const Char* name)
{
	if (m_nameAllocated && m_name != NULL)
	{
		delete[] m_name;
	}
	int liLen = (int)Strlen(name) + 1;
	m_name = new Char[liLen];
	Strcpy_s(m_name, liLen, name);
	m_nameAllocated = true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline const Char* XmlBase::getString() const
{
	return m_value;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline bool XmlBase::getBool() const
{
	return (Strcmp(m_value, T("true")) == 0 ||
		Strcmp(m_value, T("TRUE")) == 0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline int XmlBase::getInt() const
{
	return StrToI(m_value);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline unsigned long XmlBase::getHex() const
{
	unsigned long value = 0;
	/*Sscanf(m_value, T("%X"), &value);
	if (value == 0)
	{
		Sscanf(m_value, T("%x"), &value);
	}*/
	return value;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline float XmlBase::getFloat() const
{
	return static_cast<float>(StrToF(m_value));
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline double XmlBase::getDouble() const
{
	return StrToF(m_value);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline void XmlBase::setString(const Char* value)
{
	if (m_valueAllocated && m_value != NULL)
	{
		delete[] m_value;
	}
	int liLen = (int)Strlen(value) + 1;
	m_value = new Char[liLen];
	Strcpy_s(m_value, liLen, value);
	m_valueAllocated = true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline void XmlBase::setString(const String& value)
{
	setString(value.c_str());
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline void XmlBase::setBool(bool value)
{
	setString(value ? T("true") : T("false"));
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline void XmlBase::setInt(int value)
{
	Char sz[128];
	Sprintf(sz, 128, T("%d"), value);
	setString(sz);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline void XmlBase::setHex(unsigned long value)
{
	Char sz[128];
	Sprintf(sz, 128, T("%X"), value);
	setString(sz);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline void XmlBase::setFloat(float value)
{
	Char sz[128];
	Sprintf(sz, 128, T("%g"), value);
	setString(sz);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline void XmlBase::setDouble(double value)
{
	Char sz[128];
	Sprintf(sz, 128, T("%g"), value);   //change by xingej1
	setString(sz);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline NodeType XmlNode::getType() const
{
	return m_type;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline bool XmlNode::isEmpty() const
{
	return (!hasChild() && (m_value == NULL || m_value[0] == 0));
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline bool XmlNode::hasChild() const
{
	return !m_children.empty();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline XmlNode* XmlNode::getParent() const
{
	return m_parent;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
XmlNode* XmlNode::getFirstChild(NodeIterator& iter) const
{
	iter = m_children.begin();
	if (iter != m_children.end())
	{
		return *iter;
	}
	return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
XmlNode* XmlNode::getNextChild(NodeIterator& iter) const
{
	if (iter != m_children.end())
	{
		++iter;
		if (iter != m_children.end())
		{
			return *iter;
		}
	}
	return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline XmlNode* XmlNode::getChild(NodeIterator iter) const
{
	if (iter != m_children.end())
	{
		return *iter;
	}
	return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline size_t XmlNode::getChildCount() const
{
	return m_children.size();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline bool XmlNode::hasAttribute() const
{
	return !m_attributes.empty();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline XmlAttribute* XmlNode::getFirstAttribute(AttributeIterator& iter) const
{
	iter = m_attributes.begin();
	if (iter != m_attributes.end())
	{
		return *iter;
	}
	return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline XmlAttribute* XmlNode::getNextAttribute(AttributeIterator& iter) const
{
	if (iter != m_attributes.end())
	{
		++iter;
		if (iter != m_attributes.end())
		{
			return *iter;
		}
	}
	return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void XmlBase::assignString(Char* &str, Char* buffer, size_t length, bool transferCharacter)
{
#if !defined(SLIM_TRANSFER_CHARACTER)
	str = buffer;
	str[length] = 0;
#else
	const Char* found = NULL;
	if (!transferCharacter || (found = Memchr(buffer, T('&'), length)) == NULL)
	{
		str = buffer;
		str[length] = 0;
		return;
	}
	String temp;
	for (; found != NULL; found = Memchr(buffer, T('&'), length))
	{
		temp.append(buffer, found - buffer);
		length -= (found - buffer + 1);
		buffer = found + 1;
		if (length >= 5)
		{
			if (Strncmp(buffer, T("quot"), 4) == 0)
			{
				buffer += 4;
				length -= 4;
				temp.append(1, T('\"'));
				continue;
			}
			if (Strncmp(found + 1, T("apos"), 4) == 0)
			{
				buffer += 4;
				length -= 4;
				temp.append(1, T('\''));
				continue;
			}
		}
		if (length >= 4)
		{
			if (Strncmp(buffer, T("amp"), 3) == 0)
			{
				buffer += 3;
				length -= 3;
				temp.append(1, T('&'));
				continue;
			}
		}
		if (length >= 3)
		{
			if (Strncmp(buffer, T("lt"), 2) == 0)
			{
				buffer += 2;
				length -= 2;
				temp.append(1, T('<'));
				continue;
			}
			else if (Strncmp(buffer, T("gt"), 2) == 0)
			{
				buffer += 2;
				length -= 2;
				temp.append(1, T('>'));
				continue;
			}
		}
		temp.append(1, T('&'));
	}
	temp.append(buffer, length);
	size_t actualLength = temp.length();
	memcpy(str, temp.c_str(), sizeof(Char) * actualLength);
	str[actualLength] = 0;
#endif
}

///////////////////////////////////////////////////////////////////////////////////////////////////
XmlNode::XmlNode(NodeType type, XmlNode* parent)
	: m_type(type)
	, m_parent(parent)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
XmlNode::~XmlNode()
{
	clearAttribute();
	clearChild();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
XmlNode* XmlNode::findChild(const Char* name) const
{
	assert(name != NULL);
	for (NodeList::const_iterator iter = m_children.begin();
		iter != m_children.end();
		++iter)
	{
		XmlNode* child = *iter;
		assert(child != NULL);
		if (Strcmp(child->m_name, name) == 0)
		{
			return child;
		}
	}
	return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
XmlNode* XmlNode::findFirstChild(const Char* name, NodeIterator& iter) const
{
	assert(name != NULL);
	iter = m_children.begin();
	while (iter != m_children.end())
	{
		XmlNode* child = *iter;
		assert(child != NULL);
		if (Strcmp(child->m_name, name) == 0)
		{
			return child;
		}
		++iter;
	}
	return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
XmlNode* XmlNode::findNextChild(const Char* name, NodeIterator& iter) const
{
	assert(name != NULL);
	if (iter != m_children.end())
	{
		while (++iter != m_children.end())
		{
			XmlNode* child = *iter;
			assert(child != NULL);
			if (Strcmp(child->m_name, name) == 0)
			{
				return child;
			}
		}
	}
	return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
size_t XmlNode::getChildCount(const Char* name) const
{
	assert(name != NULL);

	size_t count = 0;
	for (NodeIterator iter = m_children.begin();
		iter != m_children.end();
		++iter)
	{
		XmlNode* child = *iter;
		assert(child != NULL);
		if (Strcmp(child->m_name, name) == 0)
		{
			++count;
		}
	}
	return count;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool XmlNode::removeChild(XmlNode* node)
{
	assert(node != NULL);
	for (NodeList::iterator iter = m_children.begin();
		iter != m_children.end();
		++iter)
	{
		if (*iter == node)
		{
			delete node;
			m_children.erase(iter);
			return true;
		}
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void XmlNode::clearChild()
{
	for (NodeList::iterator iter = m_children.begin();
		iter != m_children.end();
		++iter)
	{
		XmlNode* child = *iter;
		assert(child != NULL);
		delete child;
	}
	m_children.clear();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
XmlNode* XmlNode::addChild(const Char* name, NodeType type)
{
	if (type != COMMENT && type != ELEMENT)
	{
		return NULL;
	}
	XmlNode* child = new XmlNode(type, this);
	if (name != NULL)
	{
		child->setName(name);
	}
	m_children.push_back(child);
	return child;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
XmlAttribute* XmlNode::addAttribute(const Char* name, const Char* value)
{
	XmlAttribute* attribute = new XmlAttribute;
	if (name != NULL)
	{
		attribute->setName(name);
	}
	if (value != NULL)
	{
		attribute->setString(value);
	}
	m_attributes.push_back(attribute);
	return attribute;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
XmlAttribute* XmlNode::addAttribute(const Char* name, bool value)
{
	XmlAttribute* attribute = addAttribute(name);
	attribute->setBool(value);
	return attribute;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
XmlAttribute* XmlNode::addAttribute(const Char* name, int value)
{
	XmlAttribute* attribute = addAttribute(name);
	attribute->setInt(value);
	return attribute;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
XmlAttribute* XmlNode::addAttribute(const Char* name, float value)
{
	XmlAttribute* attribute = addAttribute(name);
	attribute->setFloat(value);
	return attribute;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
XmlAttribute* XmlNode::addAttribute(const Char* name, double value)
{
	XmlAttribute* attribute = addAttribute(name);
	attribute->setDouble(value);
	return attribute;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
XmlAttribute* XmlNode::findAttribute(const Char* name) const
{
	for (AttributeList::const_iterator iter = m_attributes.begin();
		iter != m_attributes.end();
		++iter)
	{
		XmlAttribute* attribute = *iter;
		assert(attribute != NULL);
		if (Strcmp(attribute->getName(), name) == 0)
		{
			return attribute;
		}
	}
	return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
const Char* XmlNode::readAttributeAsString(const Char* name, const Char* defaultValue) const
{
	XmlAttribute* attribute = findAttribute(name);
	if (attribute == NULL)
	{
		return defaultValue;
	}
	return attribute->getString();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool XmlNode::readAttributeAsBool(const Char* name, bool defaultValue) const
{
	XmlAttribute* attribute = findAttribute(name);
	if (attribute == NULL)
	{
		return defaultValue;
	}
	return attribute->getBool();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int XmlNode::readAttributeAsInt(const Char* name, int defaultValue) const
{
	XmlAttribute* attribute = findAttribute(name);
	if (attribute == NULL)
	{
		return defaultValue;
	}
	return attribute->getInt();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
unsigned long XmlNode::readAttributeAsHex(const Char* name, unsigned long defaultValue) const
{
	XmlAttribute* attribute = findAttribute(name);
	if (attribute == NULL)
	{
		return defaultValue;
	}
	return attribute->getHex();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
float XmlNode::readAttributeAsFloat(const Char* name, float defaultValue) const
{
	XmlAttribute* attribute = findAttribute(name);
	if (attribute == NULL)
	{
		return defaultValue;
	}
	return attribute->getFloat();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
double XmlNode::readAttributeAsDouble(const Char* name, double defaultValue) const
{
	XmlAttribute* attribute = findAttribute(name);
	if (attribute == NULL)
	{
		return defaultValue;
	}
	return attribute->getDouble();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void XmlNode::removeAttribute(XmlAttribute* attribute)
{
	assert(attribute != NULL);
	for (AttributeList::iterator iter = m_attributes.begin();
		iter != m_attributes.end();
		++iter)
	{
		if (*iter == attribute)
		{
			delete attribute;
			m_attributes.erase(iter);
			return;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void XmlNode::clearAttribute()
{
	for (AttributeList::iterator iter = m_attributes.begin();
		iter != m_attributes.end();
		++iter)
	{
		delete *iter;
	}
	m_attributes.clear();
}
////////////////////////////////////
int XmlNode::GetXmlBuffer(char * npBuffer, int nLen)
{
	String lStringBuffer;

	writeNode(lStringBuffer, 0);

	size_t tempBufferSize = lStringBuffer.size() * 4;	//up to 4 bytes per character
	char* tempBuffer = new char[tempBufferSize];
	size_t converted = utf16toutf8(lStringBuffer.c_str(), lStringBuffer.size(), tempBuffer, tempBufferSize);


	if (npBuffer == 0)
	{
		delete[] tempBuffer;
		return (int)converted;
	}
	else
	{
		if (nLen >= converted)
		{
			// CheckMarx fix by zhuhl5
			memcpy_s(npBuffer, nLen, tempBuffer, converted);
			delete[] tempBuffer;
			return (int)converted;
		}
		else
		{
			delete[] tempBuffer;
			return 0;
		}
	}

}



bool XmlNode::addChildNode(XmlNode* npNode)
{
	if (npNode == NULL)
		return false;

	m_children.push_back(npNode);
	return FALSE;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
void XmlNode::writeNode(String& output, int depth) const
{
	if (depth < 0)
	{
		//root node is not a real node, it has nothing to write
		return writeChildNodes(output, depth);
	}
	//add tabs
	int iTabCount = depth;
	while (iTabCount-- > 0)
	{
		output += T('	');
	}
	//comment node is special. it has no children, and has no attributes either
	if (m_type == COMMENT)
	{
		output += T("<!--");
		output += m_name;
		output += T("-->\r\n");
		return;
	}
	//start label
	output += T('<');
	writeTransferredString(output, m_name);
	//write attributes
	for (AttributeList::const_iterator iter = m_attributes.begin();
		iter != m_attributes.end();
		++iter)
	{
		XmlAttribute* attribute = *iter;
		assert(attribute != NULL);
		output += T(' ');
		writeTransferredString(output, attribute->getName());
		output += T("=\"");
		writeTransferredString(output, attribute->getString());
		output += T('"');
	}

	if (isEmpty())
	{
		output += T("/>\r\n");
		return;
	}
	output += T(">");

	if (hasChild())
	{
		output += T("\r\n");
		writeChildNodes(output, depth);
		iTabCount = depth;
		while (iTabCount-- > 0)
		{
			output += T('	');
		}
	}
	else
	{
		//leaf node
		writeTransferredString(output, m_value);
	}
	//end label
	output += T("</");
	writeTransferredString(output, m_name);
	output += T(">\r\n");
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void XmlNode::writeChildNodes(String& output, int depth) const
{
	for (NodeList::const_iterator iter = m_children.begin();
		iter != m_children.end();
		++iter)
	{
		XmlNode* child = *iter;
		assert(child != NULL);
		child->writeNode(output, depth + 1);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void XmlNode::writeTransferredString(String& output, const Char* input) const
{
	if (input == NULL)
	{
		return;
	}
#if !defined (SLIM_TRANSFER_CHARACTER)
	output += input;
#else
	for (; *input != 0; ++input)
	{
		if (*input == T('<'))
		{
			output += T("&lt");
		}
		else if (*input == T('>'))
		{
			output += T("&gt");
		}
		else if (*input == T('&'))
		{
			output += T("&amp");
		}
		else if (*input == T('\"'))
		{
			output += T("&quot");
		}
		else if (*input == T('\''))
		{
			output += T("&apos");
		}
		else
		{
			output += *input;
		}
	}
#endif
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//class CXmlDocument
///////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
XmlDocument::XmlDocument()
	: XmlNode(DOCUMENT, NULL)
	, m_buffer(NULL)
{
	m_filename[0] = UNICODE_NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
XmlDocument::~XmlDocument()
{
	if (m_buffer != NULL)
	{
		delete[] m_buffer;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool XmlDocument::loadFromFile(const Char* filename)
{
	assert(filename != NULL);
	Strcpy_s(m_filename, MAX_PATH, filename);

	std::fstream file;
	file.open(filename, std::ios_base::in | std::ios_base::binary);
	if (!file.is_open())
	{
		return false;
	}
	bool succeeded = loadFromStream(file);
	file.close();
	return succeeded;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool XmlDocument::loadFromStream(std::istream& input)
{
	input.seekg(0, std::ios::end);
	size_t size = static_cast<size_t>(input.tellg());
	input.seekg(0, std::ios::beg);

	char* buffer = new char[size];
	input.read(buffer, size);

	bool succeeded = reallyLoadFromMemory(buffer, size, true);
	if (!succeeded)
	{
		clearChild();
		if (m_buffer != NULL)
		{
			delete[] m_buffer;
			m_buffer = NULL;
		}
	}
	return succeeded;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool XmlDocument::loadFromMemory(const char* buffer, size_t size)
{
	return reallyLoadFromMemory(const_cast<char*>(buffer), size, false);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool XmlDocument::reallyLoadFromMemory(char* buffer, size_t size, bool copiedMemory)
{
	clearChild();
	if (m_buffer != NULL)
	{
		delete[] m_buffer;
		m_buffer = NULL;
	}
	if (size < 3)
	{
		return false;
	}
	//get encode
	Encode encode = ANSI;
	bool multiBytes = false;
	const unsigned char* bom = reinterpret_cast<const unsigned char*>(buffer);
	if (bom[0] == 0xfe && bom[1] == 0xff)
	{
		encode = UTF_16_BIG_ENDIAN;
	}
	else if (bom[0] == 0xff && bom[1] == 0xfe)
	{
		encode = UTF_16;
	}
	else if (bom[0] == 0xef && bom[1] == 0xbb && bom[2] == 0xbf)
	{
		encode = UTF_8;
	}
	else
	{
		encode = detectEncode(buffer, size, multiBytes);
	}

	Char* text;	//for parser
	size_t characterCount;

#ifdef SLIM_USE_WCHAR
	if (encode == UTF_16)
	{
		//skip bom
		characterCount = (size - 2) / 2;
		if (copiedMemory)
		{
			m_buffer = buffer;
		}
		else
		{
			m_buffer = new char[size];
			memcpy(m_buffer, buffer, size);
		}
		text = reinterpret_cast<wchar_t*>(buffer + 2);
	}
	else if (encode == UTF_16_BIG_ENDIAN)
	{
		//swap. can be faster
		characterCount = (size - 2) / 2;
		m_buffer = new char[characterCount * sizeof(wchar_t)];
		const wchar_t* src = reinterpret_cast<const wchar_t*>(buffer + 2);
		const wchar_t* srcEnd = src + characterCount;
		wchar_t* dst = (wchar_t*)m_buffer;
		for (; src < srcEnd; ++src, ++dst)
		{
			*((char*)dst) = *(((const char*)src) + 1);
			*(((char*)dst) + 1) = *((const char*)src);
		}
		text = (wchar_t*)m_buffer;
		if (copiedMemory)
		{
			delete[] buffer;
		}
	}
	else if (encode == UTF_8 || encode == UTF_8_NO_MARK || (encode == ANSI && !multiBytes))
	{
		m_buffer = new char[size * sizeof(wchar_t)];
		text = (wchar_t*)m_buffer;
		if (encode == UTF_8)
		{
			//skip bom
			characterCount = utf8toutf16(buffer + 3, size - 3, text, size);
		}
		else
		{
			characterCount = utf8toutf16(buffer, size, text, size);
		}
		if (copiedMemory)
		{
			delete[] buffer;
		}
	}
	else
	{
		if (copiedMemory)
		{
			delete[] buffer;
		}
		return false;
	}
#elif defined(UNICODE)
	if (encode == UTF_8)
	{
		//skip bom
		characterCount = size - 3;
		if (copiedMemory)
		{
			m_buffer = buffer;
		}
		else
		{
			m_buffer = new char[size];
			memcpy(m_buffer, buffer, size);
		}
		text = m_buffer + 3;
	}
	else if (encode == UTF_16)
	{
		characterCount = (size - 2) / 2;
		m_buffer = new char[characterCount * 4];
		//it's not really character count here
		characterCount = utf16toutf8(reinterpret_cast<const wchar_t*>(buffer + 2), characterCount, m_buffer, characterCount * 4);
		text = m_buffer;
		if (copiedMemory)
		{
			delete[] buffer;
		}
	}
	else if (encode == UTF_16_BIG_ENDIAN)
	{
		characterCount = (size - 2) / 2;
		//swap
		wchar_t* leBuffer = new wchar_t[characterCount];
		const wchar_t* src = reinterpret_cast<const wchar_t*>(buffer + 2);
		const wchar_t* srcEnd = src + characterCount;
		wchar_t* dst = leBuffer;
		for (; src < srcEnd; ++src, ++dst)
		{
			*((char*)dst) = *(((const char*)src) + 1);
			*(((char*)dst) + 1) = *((const char*)src);
		}
		m_buffer = new char[characterCount * 4];
		//it's not really character count here
		characterCount = utf16toutf8(leBuffer, characterCount, m_buffer, characterCount * 4);
		text = m_buffer;
		delete[] leBuffer;
		if (copiedMemory)
		{
			delete[] buffer;
		}
	}
	else if (encode == UTF_8_NO_MARK)
	{
		characterCount = size;
		if (copiedMemory)
		{
			m_buffer = buffer;
		}
		else
		{
			m_buffer = new char[size];
			memcpy(m_buffer, buffer, size);
		}
		text = m_buffer;
	}
	else
	{
		if (copiedMemory)
		{
			delete[] buffer;
		}
		return false;
	}
#else
	if (encode != ANSI)
	{
		return false;
	}
	characterCount = size;
	text = static_cast<char*>(buffer);
#endif	//#ifdef SLIM_USE_WCHAR

	return parse(text, characterCount);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool XmlDocument::save(const Char* filename, Encode encode) const
{
	if (filename == NULL)
		filename = m_filename;

	assert(filename != NULL);

	String output;

	encode = UTF_16;

#ifdef SLIM_USE_WCHAR
	if (encode == ANSI)
	{
		return false;
	}
	if (encode == UTF_16 || encode == UTF_16_BIG_ENDIAN)
	{
		output = T("<?xml version=\"1.0\" encoding=\"UTF-16\"?>\r\n");
	}
	else if (encode == UTF_8 || encode == UTF_8_NO_MARK)
	{
		output = T("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n");
	}
#elif defined (UNICODE)
	if (encode != UTF_8 && encode != UTF_8_NO_MARK)
	{
		return false;
	}
	output = T("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n");
#else
	if (encode != ANSI)
	{
		return false;
	}
	//don't know how to get encode string of current code page
	output = T("<?xml version=\"1.0\"?>\r\n");
#endif

	writeNode(output, -1);

	std::fstream file;
	file.open(filename, std::ios_base::out | std::ios_base::binary);
	if (!file.is_open())
	{
		return false;
	}

#ifdef SLIM_USE_WCHAR
	unsigned char bom[3];
	if (encode == UTF_16)
	{
		bom[0] = 0xff;
		bom[1] = 0xfe;
		file.write((char*)bom, 2);
		file.write((char*)output.c_str(), output.size() * 2);
	}
	else if (encode == UTF_16_BIG_ENDIAN)
	{
		bom[0] = 0xfe;
		bom[1] = 0xff;
		file.write((char*)bom, 2);
		//swap
		wchar_t* buffer = new wchar_t[output.size()];
		const wchar_t* src = output.c_str();
		const wchar_t* srcEnd = src + output.size();
		wchar_t* dst = buffer;
		for (; src < srcEnd; ++src, ++dst)
		{
			*((char*)dst) = *(((const char*)src) + 1);
			*(((char*)dst) + 1) = *((const char*)src);
		}
		file.write((char*)buffer, output.size() * 2);
		delete[] buffer;
	}
	else if (encode == UTF_8 || encode == UTF_8_NO_MARK)
	{
		if (encode == UTF_8)
		{
			bom[0] = 0xef;
			bom[1] = 0xbb;
			bom[2] = 0xbf;
			file.write((char*)bom, 3);
		}
		size_t tempBufferSize = output.size() * 4;	//up to 4 bytes per character
		char* tempBuffer = new char[tempBufferSize];
		size_t converted = utf16toutf8(output.c_str(), output.size(), tempBuffer, tempBufferSize);
		file.write(tempBuffer, converted);
		delete[] tempBuffer;
	}
#else
#if defined (UNICODE)
	if (encode == UTF_8)
	{
		unsigned char bom[3];
		bom[0] = 0xef;
		bom[1] = 0xbb;
		bom[2] = 0xbf;
		file.write((char*)bom, 3);
	}
#endif
	file.write(output.c_str(), output.size());
#endif

	file.close();
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool XmlDocument::parse(Char* input, size_t size)
{
	Char* cur = input;
	Char* end = input + size;

	Char* label = NULL;
	size_t labelSize = 0;
	int depth = 0;
	XmlNode* currentNode = this;

	while (cur < end)
	{
		assert(depth >= 0);
		assert(currentNode != NULL);

		Char* lastPos = cur;
		if (!findLabel(cur, end - cur, label, labelSize))
		{
			break;
		}
		switch (*label)
		{
		case T('/'):	//node ending
			if (depth < 1)
			{
				return false;
			}
			if (currentNode->getType() == ELEMENT && !currentNode->hasChild())
			{
				currentNode->assignString(currentNode->m_value, lastPos, label - lastPos - 1, true);
			}
			currentNode = currentNode->getParent();
			--depth;
			break;
		case T('?'):	//xml define node, ignore
			break;
		case T('!'):	//comment node
		{
			//ignore !-- and --
			if (labelSize < 5)
			{
				return false;
			}
			XmlNode* comment = currentNode->addChild(NULL, COMMENT);
			comment->assignString(comment->m_name, label + 3, labelSize - 5, false);
		}
		break;
		default:	//node start
		{
			XmlNode* newNode = currentNode->addChild(NULL, ELEMENT);

			bool emptyNode = parseLabel(newNode, label, labelSize);

			if (!emptyNode)
			{
				currentNode = newNode;
				++depth;
			}
		}
		}
	} // while(cur < end)

	if (depth != 0)
	{
		return false;
	}
	assert(currentNode == this);
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool XmlDocument::findLabel(Char* &begin, size_t size, Char* &label, size_t &labelSize)
{
	label = (Char*)Memchr(begin, T('<'), size);
	if (label == NULL)
	{
		return false;
	}
	++label;
	size -= (label - begin);

	//comment is special, won't end without "-->"
	if (size > 6 //Strlen(T("!---->"))
		&& label[0] == T('!')
		&& label[1] == T('-')
		&& label[2] == T('-'))
	{
		//buffer is not NULL-terminated, so we can't use strstr, shit! is there a "safe" version of strstr?
		Char* cur = label + 3;	//skip !--
		size -= 5; //(Strlen(T("!---->")) - 1);
		while (true)
		{
			Char* end = (Char*)Memchr(cur, T('-'), size);
			if (end == NULL)
			{
				return false;
			}
			if (*(end + 1) == T('-') && *(end + 2) == T('>'))
			{
				//get it
				labelSize = end - label + 2;
				begin = end + 3;
				return true;
			}
			size -= (end - cur + 1);
			cur = end + 1;
		}
	}
	begin = (Char*)Memchr(label, T('>'), size);
	if (begin == NULL)
	{
		return false;
	}
	labelSize = begin - label;
	++begin;
	if (labelSize == 0)
	{
		return false;
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool XmlDocument::parseLabel(XmlNode* node, Char* label, size_t labelSize)
{
	//get name
	Char* cur = label;
	while (*cur != T(' ') && *cur != T('/') && *cur != T('>'))
	{
		++cur;
	}
	Char next = *cur;
	node->assignString(node->m_name, label, cur - label, true);
	if (next != T(' '))
	{
		return next == T('/');
	}
	//get attributes
	Char* end = label + labelSize;
	++cur;
	while (cur < end)
	{
		while (*cur == T(' '))
		{
			++cur;
		}
		//attribute name
		Char* attrName = cur;
		while (*cur != T(' ') && *cur != T('=') && *cur != T('/') && *cur != T('>'))
		{
			++cur;
		}
		next = *cur;
		size_t attrNameSize = cur - attrName;

		//attribute value
		cur = (Char*)Memchr(cur, T('"'), end - cur);
		if (NULL == cur)
		{
			break;
		}
		Char* attrValue = ++cur;
		cur = (Char*)Memchr(cur, T('"'), end - cur);
		if (NULL == cur)
		{
			return false;
		}
		size_t attrValueSize = cur - attrValue;

		XmlAttribute* attribute = node->addAttribute();
		attribute->assignString(attribute->m_name, attrName, attrNameSize, true);
		attribute->assignString(attribute->m_value, attrValue, attrValueSize, true);
		++cur;
	}
	return next == T('/');
}

///////////////////////////////////////////////////////////////////////////////////////////////////
size_t utf8toutf16(const char* u8, size_t size, wchar_t* u16, size_t outBufferSize)
{
	size_t converted = 0;

	while (size > 0)
	{
		if ((*u8 & 0x80) == 0)
		{
			*(u16++) = *(u8++);
			--size;
			++converted;
		}
		else if ((*u8 & 0xe0) == 0xc0)
		{
			if (size < 2)
			{
				break;
			}
			*(u16++) = (*u8 & 0x1f) | ((*(u8 + 1) & 0x3f) << 5);
			u8 += 2;
			size -= 2;
			++converted;
		}
		else if ((*u8 & 0xf0) == 0xe0)
		{
			if (size < 3)
			{
				break;
			}
			*u16 = ((*u8 & 0x0f) << 12) | ((*(u8 + 1) & 0x3f) << 6) | (*(u8 + 2) & 0x3f);
			u8 += 3;
			++u16;
			size -= 3;
			++converted;
		}
		else
		{
			break;
		}
		if (converted == outBufferSize)
		{
			break;
		}
	}
	return converted;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
size_t utf16toutf8(const wchar_t* u16, size_t size, char* u8, size_t outBufferSize)
{
	size_t converted = 0;

	while (size > 0)
	{
		if (*u16 < 0x80)
		{
			//1 byte
			if (converted == outBufferSize)
			{
				break;
			}
			*(u8++) = static_cast<char>(*(u16++));
			--size;
			++converted;
		}
		else if (*u16 < 0x800)
		{
			//2 bytes
			if (converted + 2 > outBufferSize)
			{
				break;
			}
			*u8 = (*u16 >> 6) | 0xc0;
			*(u8 + 1) = (*u16 & 0x3f) | 0x80;
			u8 += 2;
			++u16;
			--size;
			converted += 2;
		}
		else
		{
			//3 bytes
			if (converted + 3 > outBufferSize)
			{
				break;
			}
			*u8 = (*u16 >> 12) | 0xe0;
			*(u8 + 1) = ((*u16 >> 6) & 0x3f) | 0x80;
			*(u8 + 2) = (*u16 & 0x3f) | 0x80;
			u8 += 3;
			++u16;
			--size;
			converted += 3;
		}
	}
	return converted;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
Encode detectEncode(const char* str, size_t size, bool& multiBytes)
{
	while (size > 0)
	{
		if ((*str & 0x80) == 0)
		{
			//1 byte
			++str;
			--size;
		}
		else
		{
			multiBytes = true;
			if ((*str & 0xf0) == 0xe0)
			{
				//3 bytes
				if (size < 3)
				{
					return ANSI;
				}
				if ((*(str + 1) & 0xc0) != 0x80 || (*(str + 2) & 0xc0) != 0x80)
				{
					return ANSI;
				}
				str += 3;
				size -= 3;
			}
			else if ((*str & 0xe0) == 0xc0)
			{
				//2 bytes
				if (size < 2)
				{
					return ANSI;
				}
				if ((*(str + 1) & 0xc0) != 0x80)
				{
					return ANSI;
				}
				str += 2;
				size -= 2;
			}
			else if ((*str & 0xf8) == 0xf0)
			{
				//4 bytes
				if (size < 4)
				{
					return ANSI;
				}
				if ((*(str + 1) & 0xc0) != 0x80 || (*(str + 2) & 0xc0) != 0x80 || (*(str + 3) & 0xc0) != 0x80)
				{
					return ANSI;
				}
				str += 4;
				size -= 4;
			}
			else
			{
				return ANSI;
			}
		}
	}
#if defined(UNICODE)
	return UTF_8_NO_MARK;
#else
	return multiBytes ? UTF_8_NO_MARK : ANSI;
#endif
}

//}
