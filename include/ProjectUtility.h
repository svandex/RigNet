#ifndef __PROJECTUTILITY_H__
#define __PROJECTUTILITY_H__
#include "HeaderPrecompilation.h"

namespace TestValidation
{
enum ChunkType
{
	Normal,
	File,
	EndOfChunk
};

class Chunk
{
public:
	Chunk() = delete;
	Chunk(const Chunk &) = delete;

	//初始位置指针，不删除该指针
	//该指针一定是在Chunk析构之前一直存在
	Chunk(void *, unsigned long);

	//数据指针
	void *data();

	//数据大小
	unsigned long size()
	{
		return _size_of_data;
	}

	//元信息
	rapidjson::Value metadata();

	//类型
	ChunkType type;

private:
	//chunk大小
	unsigned long _size_of_chunk = 0;
	//元信息开始位置偏移量
	unsigned long _sof_metadata = 0;
	//元信息结束位置偏移量
	unsigned long _eof_metadata = 0;
	//数据大小
	unsigned long _size_of_data = 0;
};

class ChunkParser
{
public:
	ChunkParser() = delete;
	ChunkParser(const ChunkParser &) = delete;
	ChunkParser(void *, unsigned long);

	rapidjson::Document metadata();

	//将文件存储到特定路径下
	bool store(size_t, const char *);

private:
	std::vector<Chunk> _vector_of_chunk;
};

} // namespace TestValidation

#endif