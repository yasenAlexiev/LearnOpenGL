#pragma once

class IndexBuffer
{

private:
	unsigned int m_RendererID;
	unsigned int count;
public:
	IndexBuffer( const unsigned int* data, unsigned int count );
	~IndexBuffer();

	void Bind() const;
	void Unbined() const;

	unsigned int GetCount() const
	{
		return count;
	}
};